#include <algorithm>
#include <atomic>
#include <cassert>
#include <utility>

#include <zest/math/math_utils.h>
#include <zest/string/murmur_hash.h>
#include <zest/time/pico_profiler.h>
#include <format>

using namespace std::chrono;
using namespace Zest;

PicoMutex gMutex;

// Initial prototypes used:
// https://gist.github.com/CedricGuillemet/fc82e7a9b2f703ff1ebf
// https://github.com/arnaud-jamin/imgui/blob/master/profiler.cpp
//
// This one is better at spans that cover frame boundaries, uses cross platform cpp libraries
// instead of OS specific ones.
// No claims made about performance, but in practice has been very useful in finding bugs
// Has a summary view and support for zoom/pan, CTRL+select range or click in the summary graphs
// There is an optional single 'Region' which I use for audio frame profiling; you can only have one
// The profile macros can pick a unique/nice color for a given name.
// There is a LOCK_GUARD wrapper around a mutex, for tracking lock times
// Requires some helper code from my zest library (https://github.com/Rezonality/Zest) for:
// - NVec/NRect
// - murmur hash (for text to color)
// - dpi
// - fmt for text formatting
// I typically use this as a 'one shot, collect frames' debugger.  I hit the Pause/Resume button at interesting spots and collect a bunch of frames.
// You have to pause to navigate/inspect.
// All memory allocation is up-front; change the 'Max' values below to collect more frames
// The profiler just 'stops' when the memory is full.  It can be restarted/stopped.
// I pulled this together over the space of a weekend, it could be tidier here and there, but it works great ;)
namespace Zest
{

namespace Profiler
{

ProfileSettings settings;

namespace
{
Zest::timer gTimer;
bool gPaused = true;
bool gRequestPause = false;
bool gRestarting = true;

std::atomic<uint64_t> gProfilerGeneration = 0;
thread_local int gThreadIndexTLS = -1;
thread_local uint64_t gGenerationTLS = -1;

std::vector<ThreadData> gThreadData;
std::vector<Frame> gFrameData;
std::vector<Region> gRegionData;

int64_t gMaxFrameTime = duration_cast<nanoseconds>(milliseconds(20)).count();
uint32_t gCurrentFrame = 0;

// Region
uint32_t gCurrentRegion = 0;

std::vector<glm::vec4> DefaultColors;

} // namespace

void Reset();

// Optionally call this before doing any profiler calls to change the defaults
void SetProfileSettings(const ProfileSettings& s)
{
    settings = s;
    Reset();
}

#define NUM_DEFAULT_COLORS 16

void CalculateColors()
{
    double golden_ratio_conjugate = 0.618033988749895;
    double h = .85f;
    for (int i = 0; i < (int)NUM_DEFAULT_COLORS; i++)
    {
        h += golden_ratio_conjugate;
        h = std::fmod(h, 1.0);
        DefaultColors.emplace_back(HSVToRGB(float(h) * 360.0f, 0.6f, 200.0f));
    }
}

// Run Init every time a profile is started
void Init()
{
    CalculateColors();

    gThreadData.resize(settings.MaxThreads);

    gProfilerGeneration++;

    for (uint32_t iZero = 0; iZero < settings.MaxThreads; iZero++)
    {
        ThreadData* threadData = &gThreadData[iZero];
        threadData->initialized = iZero == 0;
        threadData->maxLevel = 0;
        threadData->minTime = std::numeric_limits<int64_t>::max();
        threadData->maxTime = 0;
        threadData->currentEntry = 0;
        threadData->name = std::string("Thread ") + std::to_string(iZero);
        threadData->entries.resize(settings.MaxEntriesPerThread);
        memset(threadData->entries.data(), 0, sizeof(ProfilerEntry) * threadData->entries.size());
        threadData->entryStack.resize(50);
        threadData->callStackDepth = 0;
    }

    gFrameData.resize(settings.MaxFrames);
    gRegionData.resize(settings.MaxRegions);
    for (auto& frame : gFrameData)
    {
        frame.frameThreads.resize(settings.MaxThreads);
        frame.frameThreadCount = 0;
        for (auto& threadInfo : frame.frameThreads)
        {
            threadInfo.activeEntry = 0;
            threadInfo.threadIndex = 0;
        }
    }

    gThreadIndexTLS = 0;
    gGenerationTLS = 0;
    gThreadData[0].initialized = true;
    gRestarting = true;
    gCurrentFrame = 0;
    gCurrentRegion = 0;
    gMaxFrameTime = duration_cast<nanoseconds>(milliseconds(30)).count();
    timer_start(gTimer);

    gPaused = false;
}

void InitThread()
{
    PicoLockGuard lock(gMutex);

    for (uint32_t iThread = 0; iThread < settings.MaxThreads; iThread++)
    {
        ThreadData* threadData = &gThreadData[iThread];
        if (!threadData->initialized)
        {
            // use it
            gThreadIndexTLS = iThread;
            gGenerationTLS = gProfilerGeneration;
            threadData->currentEntry = 0;
            threadData->initialized = true;

            return;
        }
    }
    assert(false && "Every thread slots are used!");
}

void FinishThread()
{
    PicoLockGuard lock(gMutex);
    assert(gThreadIndexTLS > -1 && "Trying to finish an uninitilized thread.");
    gThreadData[gThreadIndexTLS].initialized = false;
    gThreadIndexTLS = -1;
}

void Finish()
{
    gThreadData.clear();
}

void SetPaused(bool pause)
{
    if (gPaused != pause)
    {
        gRequestPause = pause;
    }
}

ThreadData* GetThreadData()
{
    if (gGenerationTLS != gProfilerGeneration.load())
    {
        gThreadIndexTLS = -1;
    }

    if (gThreadIndexTLS == -1)
    {
        InitThread();
    }

    return &gThreadData[gThreadIndexTLS];
}

void HideThread()
{
    if (gPaused)
    {
        return;
    }
    ThreadData* threadData = GetThreadData();
    threadData->hidden = true;
}

void Reset()
{
    PicoLockGuard lock(gMutex);
    gThreadIndexTLS = -1;
    Init();
}

bool CheckEndState()
{
    if (gThreadData[gThreadIndexTLS].currentEntry >= settings.MaxEntriesPerThread)
    {
        gPaused = true;
        gRequestPause = true;
    }

    if (gCurrentFrame >= settings.MaxFrames)
    {
        gPaused = true;
        gRequestPause = true;
    }

    if (gCurrentRegion >= settings.MaxRegions)
    {
        gPaused = true;
        gRequestPause = true;
    }
    return gPaused;
}

void PushSectionBase(const char* szSection, unsigned int color, const char* szFile, int line)
{
    if (gPaused)
    {
        return;
    }

    ThreadData* threadData = GetThreadData();
    if (CheckEndState())
    {
        return;
    }

    assert(threadData->callStackDepth < settings.MaxCallStack && "Might need to make call stack bigger!");

    // check again
    if (gPaused)
    {
        return;
    }

    // Write entry 0
    ProfilerEntry* profilerEntry = &threadData->entries[threadData->currentEntry];
    threadData->entryStack[threadData->callStackDepth] = threadData->currentEntry;

    if (threadData->callStackDepth > 0)
    {
        profilerEntry->parent = threadData->entryStack[threadData->callStackDepth - 1];
        assert(profilerEntry->parent < threadData->currentEntry);
    }
    else
    {
        profilerEntry->parent = 0xFFFFFFFF;
    }

    assert(szFile != NULL && "No file string specified");
    assert(szSection != NULL && "No section name specified");

    profilerEntry->color = color;
    profilerEntry->szFile = szFile;
    profilerEntry->szSection = szSection;
    profilerEntry->line = line;
    profilerEntry->startTime = timer_get_elapsed(gTimer).count();
    profilerEntry->endTime = std::numeric_limits<int64_t>::max();
    profilerEntry->level = threadData->callStackDepth;
    threadData->callStackDepth++;
    threadData->currentEntry++;

    threadData->maxLevel = std::max(threadData->maxLevel, threadData->callStackDepth);

    threadData->minTime = std::min(profilerEntry->startTime, threadData->minTime);
    threadData->maxTime = std::max(profilerEntry->startTime, threadData->maxTime);

    // New thread begin during frame
    if (threadData->currentEntry == 1)
    {
        // Add this thread to the current frame info
        if (gCurrentFrame > 0)
        {
            auto& frame = gFrameData[gCurrentFrame - 1];
            auto& threadInfo = frame.frameThreads[frame.frameThreadCount];
            threadInfo.activeEntry = threadData->currentEntry - 1;
            threadInfo.threadIndex = gThreadIndexTLS;
            frame.frameThreadCount++;
        }
    }

    if (gRestarting)
    {
        gRestarting = false;
    }
}

void PopSection()
{
    if (gPaused)
    {
        return;
    }

    ThreadData* threadData = GetThreadData();
    if (CheckEndState())
    {
        return;
    }

    if (gRestarting || gPaused)
    {
        return;
    }

    if (threadData->callStackDepth <= 0)
    {
        return;
    }

    // Back to the last entry we wrote
    threadData->callStackDepth--;

    int entryIndex = threadData->entryStack[threadData->callStackDepth];
    ProfilerEntry* profilerEntry = &threadData->entries[entryIndex];

    assert(profilerEntry->szSection != nullptr);

    // store end time
    profilerEntry->endTime = timer_get_elapsed(gTimer).count();

    threadData->maxTime = std::max(profilerEntry->endTime, threadData->maxTime);
}

void NameThread(const char* pszName)
{
    if (gPaused)
    {
        return;
    }

    // Must get thread data to init the thread
    ThreadData* threadData = GetThreadData();
    threadData->name = pszName;
}

// You are allowed one secondary region - I use it for audio thread monitoring
void BeginRegion()
{
    if (gPaused)
    {
        return;
    }

    // Must get thread data to init the thread
    ThreadData* threadData = GetThreadData();
    if (CheckEndState())
    {
        return;
    }

    auto& region = gRegionData[gCurrentRegion];
    region.startTime = timer_get_elapsed(gTimer).count();
}

void EndRegion()
{
    if (gPaused)
    {
        return;
    }

    ThreadData* threadData = GetThreadData();
    if (CheckEndState())
    {
        return;
    }

    auto& region = gRegionData[gCurrentRegion];
    region.endTime = timer_get_elapsed(gTimer).count();
    region.name = std::format("{:.2f}ms", float(timer_to_ms(nanoseconds(region.endTime - region.startTime))));

    gCurrentRegion++;
}

void NewFrame()
{
    if (gPaused)
    {
        return;
    }

    if (CheckEndState())
    {
        return;
    }

    auto& frame = gFrameData[gCurrentFrame];
    for (uint32_t threadIndex = 0; threadIndex < settings.MaxThreads; threadIndex++)
    {
        auto& thread = gThreadData[threadIndex];
        if (!thread.initialized)
        {
            continue;
        }
        if (thread.currentEntry > 0)
        {
            // Remember which entry was active for this thread
            auto& threadInfo = frame.frameThreads[frame.frameThreadCount];
            threadInfo.activeEntry = thread.currentEntry - 1;
            threadInfo.threadIndex = threadIndex;

            // Next thread
            frame.frameThreadCount++;
        }
    }

    frame.startTime = timer_get_elapsed(gTimer).count();
    if (gCurrentFrame > 0)
    {
        gFrameData[gCurrentFrame - 1].endTime = frame.startTime;
        gFrameData[gCurrentFrame - 1].name = std::format("{:.2f}ms", float(timer_to_ms(nanoseconds(frame.startTime - gFrameData[gCurrentFrame - 1].startTime))));
    }
    gCurrentFrame++;
}

const glm::vec4& ColorFromName(const char* pszName, const uint32_t len)
{
    const auto col = murmur_hash(pszName, len, 0);
    return DefaultColors[col % NUM_DEFAULT_COLORS];
}

} // namespace Profiler
} // namespace Zest
