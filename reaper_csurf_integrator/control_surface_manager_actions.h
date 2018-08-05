//
//  control_surface_manager_actions.h
//  reaper_csurf_integrator
//
//

#ifndef control_surface_manager_actions_h
#define control_surface_manager_actions_h

#include "control_surface_base_actions.h"

extern Manager* TheManager;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MapFXToWidgets  : public Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    void Do(Page* page, RealSurface* surface) override
    {
        if(1 == DAW::CountSelectedTracks(nullptr))
        {
            MediaTrack* track = nullptr;
            
            for(int i = 0; i < CSurf_NumTracks(page->GetFollowMCP()); i++)
                if(DAW::GetMediaTrackInfo_Value(DAW::CSurf_TrackFromID(i, page->GetFollowMCP()), "I_SELECTED"))
                {
                    track = DAW::CSurf_TrackFromID(i, page->GetFollowMCP());
                    break;
                }
            
            if(track)
            {
                page->UnmapWidgetsFromFX();
                page->MapFXToWidgets(track);
            }
        }
        else
            page->UnmapWidgetsFromFX();
    }
    
    void Do(Page* page, RealSurface* surface, double value) override
    {
        if(1 == DAW::CountSelectedTracks(nullptr))
        {
            MediaTrack* track = nullptr;
            
            for(int i = 0; i < CSurf_NumTracks(page->GetFollowMCP()); i++)
                if(DAW::GetMediaTrackInfo_Value(DAW::CSurf_TrackFromID(i, page->GetFollowMCP()), "I_SELECTED"))
                {
                    track = DAW::CSurf_TrackFromID(i, page->GetFollowMCP());
                    break;
                }
            
            if(track)
            {
                page->ToggleMapFXToWidgets(surface, track);
            }
        }
        else
            page->UnmapWidgetsFromFX();
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MapTrackAndFXToWidgets  : public Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    void Do(Page* page, RealSurface* surface) override
    {
        if(1 == DAW::CountSelectedTracks(nullptr))
        {
            MediaTrack* track = nullptr;
            
            for(int i = 0; i < CSurf_NumTracks(page->GetFollowMCP()); i++)
                if(DAW::GetMediaTrackInfo_Value(DAW::CSurf_TrackFromID(i, page->GetFollowMCP()), "I_SELECTED"))
                {
                    track = DAW::CSurf_TrackFromID(i, page->GetFollowMCP());
                    break;
                }
            
            if(track)
            {
                page->UnmapWidgetsFromTrackAndFX();
                page->MapTrackAndFXToWidgets(surface, track);
            }
        }
        else
            page->UnmapWidgetsFromTrackAndFX();
    }

    void Do(Page* page, RealSurface* surface, double value) override
    {
        if(1 == DAW::CountSelectedTracks(nullptr))
        {
            MediaTrack* track = nullptr;
            
            for(int i = 0; i < CSurf_NumTracks(page->GetFollowMCP()); i++)
                if(DAW::GetMediaTrackInfo_Value(DAW::CSurf_TrackFromID(i, page->GetFollowMCP()), "I_SELECTED"))
                {
                    track = DAW::CSurf_TrackFromID(i, page->GetFollowMCP());
                    break;
                }
            
            if(track)
            {
                page->ToggleMapTrackAndFXToWidgets(surface, track);
            }
        }
        else
            page->UnmapWidgetsFromTrackAndFX();
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SetShowFXWindows : public Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    void RequestUpdate(Page* page, ActionContext* actionContext, Widget* widget) override
    {
        actionContext->SetWidgetValue(widget, 0, page->GetShowFXWindows());
    }

    void Do(Page* page, double value) override
    {
        page->SetShowFXWindows(value);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CycleTimeDisplayModes : public Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    void Do(Page* page, double value) override
    {
        page->CycleTimeDisplayModes();
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Latched : public Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    void Do(Page* page, double value) override
    {
        
    }

    /*
private:
    clock_t lastPressed_ = clock();
public:
    
    virtual void SetValue(string zoneName, string surfaceName, double value) {}
    
    virtual void Do(double value, string zoneName, string surfaceName, string widgetName) override
    {
        if(value != 0)
        {
            lastPressed_ = clock();
            SetValue(zoneName, surfaceName, value);
            GetLayer()->SetWidgetValue(zoneName, surfaceName, widgetName, value);
        }
        else
        {
            if(clock() - lastPressed_ >  CLOCKS_PER_SEC / 4)
            {
                SetValue(zoneName, surfaceName, value);
                GetLayer()->SetWidgetValue(zoneName, surfaceName, widgetName, value);
            }
        }
    }
     */
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LatchedZoom : public Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    void Do(Page* page, double value) override
    {
        
    }

    /*
public:
    
    virtual void SetValue(string zoneName, string surfaceName, double value) override
    {
        GetLayer()->SetZoom(zoneName, surfaceName, value);
    }
     */
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LatchedScrub : public Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    void Do(Page* page, double value) override
    {
        
    }

    /*
public:
    
    virtual void SetValue(string zoneName, string surfaceName, double value) override
    {
        GetLayer()->SetScrub(zoneName, surfaceName, value);
    }
     */
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class NextPage : public Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    void Do(Page* page, double value) override
    {
        TheManager->NextPage();
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TrackBank : public Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    void Do(Page* page, double stride) override
    {
         TheManager->AdjustTrackBank(page, stride);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PinSelectedTracks : public Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    void Do(Page* page, double value) override
    {
        page->PinSelectedTracks();
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UnpinSelectedTracks : public Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    void Do(Page* page, double value) override
    {
        page->UnpinSelectedTracks();
    }
};

#endif /* control_surface_manager_actions_h */
