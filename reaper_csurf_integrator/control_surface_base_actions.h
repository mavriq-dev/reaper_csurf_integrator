//
//  control_surface_base_actions.h
//  reaper_csurf_integrator
//
//

#ifndef control_surface_base_actions_h
#define control_surface_base_actions_h

#include "control_surface_integrator.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GlobalContext : public ActionContext
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    GlobalContext(Action* action, bool isInverted) : ActionContext(action, isInverted) {}
    
    virtual void RequestActionUpdate(Page* page, Widget* widget) override
    {
        action_->RequestUpdate(page, this, widget);
    }
    
    virtual void DoAction(Page* page, Widget* widget, double value) override
    {
        action_->Do(page, isInverted_ == false ? value : 1.0 - value);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TrackContext : public ActionContext
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
protected:
    string trackGUID_ = "";
    
public:
    TrackContext(Action* action, bool isInverted) : ActionContext(action, isInverted) {}
    
    virtual void  SetTrack(string trackGUID) override
    {
        trackGUID_ = trackGUID;
    }
    
    virtual void RequestActionUpdate(Page* page, Widget* widget) override
    {
        if(MediaTrack* track = DAW::GetTrackFromGUID(trackGUID_, page->GetFollowMCP()))
        {
            action_->RequestUpdate(page, this, widget, track);
        }
        else
        {
            widget->SetValue(0, 0.0);
            widget->SetValue("");
        }
    }
    
    virtual void DoAction(Page* page, Widget* widget, double value) override
    {
        if(MediaTrack* track = DAW::GetTrackFromGUID(trackGUID_, page->GetFollowMCP()))
            action_->Do(page, widget, track, isInverted_ == false ? value : 1.0 - value);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TrackTouchControlledContext : public TrackContext
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    TrackTouchControlledContext(Action* action, bool isInverted) : TrackContext(action, isInverted) {}

    virtual void RequestActionUpdate(Page* page, Widget* widget) override
    {
        if(MediaTrack* track = DAW::GetTrackFromGUID(trackGUID_, page->GetFollowMCP()))
        {
            if(page->GetTouchState(track, 0))
            {
                action_->RequestUpdate(page, this, widget, track);
            }
        }
        else
        {
            widget->SetValue(0, 0.0);
            widget->SetValue("");
        }
    }
    
    virtual void DoAction(Page* page, Widget* widget, double value) override
    {
        if(MediaTrack* track = DAW::GetTrackFromGUID(trackGUID_, page->GetFollowMCP()))
            if(page->GetTouchState(track, 0))
                action_->Do(page, widget, track, isInverted_ == false ? value : 1.0 - value);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TrackContextWithIntParam : public TrackContext
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    int param_ = 0;
    
public:
    TrackContextWithIntParam(Action* action, int param, bool isInverted) : TrackContext(action, isInverted), param_(param) {}
    
    virtual void RequestActionUpdate(Page* page, Widget* widget) override
    {
        if(MediaTrack* track = DAW::GetTrackFromGUID(trackGUID_, page->GetFollowMCP()))
        {
            action_->RequestUpdate(page, this, widget, track, param_);
        }
        else
        {
            widget->SetValue(0, 0.0);
            widget->SetValue("");
        }
    }
    
    virtual void DoAction(Page* page, Widget* widget, double value) override
    {
        action_->Do(page, param_);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FXContext : public TrackContext
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    string fxParamName_ = "";
    int fxIndex_ = 0;
    
public:
    FXContext(Action* action, string fxParamName, bool isInverted) : TrackContext(action, isInverted), fxParamName_(fxParamName) {}
    
    void SetIndex(int index) override { fxIndex_ = index; }
    
    virtual void RequestActionUpdate(Page* page, Widget* widget) override
    {
        if(MediaTrack* track = DAW::GetTrackFromGUID(trackGUID_, page->GetFollowMCP()) )
            action_->RequestUpdate(this, widget, track, fxIndex_, page->GetFXParamIndex(track, widget, fxIndex_, fxParamName_));
    }
    
    virtual void DoAction(Page* page, Widget* widget, double value) override
    {
        if(MediaTrack* track = DAW::GetTrackFromGUID(trackGUID_, page->GetFollowMCP()))
            action_->Do(track, fxIndex_, page->GetFXParamIndex(track, widget, fxIndex_, fxParamName_), value);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ReaperActionContext : public ActionContext
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    int commandId_ = 0;
    
public:
    ReaperActionContext(Action* action, string commandStr, bool isInverted) : ActionContext(action, isInverted)
    {
        commandId_ =  atol(commandStr.c_str());
        
        if(commandId_ == 0) // unsuccessful conversion to number
        {
        
        commandId_ = DAW::NamedCommandLookup(commandStr.c_str()); // look up by string
        
        if(commandId_ == 0) // can't find it
            commandId_ = 65535; // no-op
        }
    }
    
    virtual void RequestActionUpdate(Page* page, Widget* widget) override
    {
        action_->RequestUpdate(page, this, widget, commandId_);
    }
    
    virtual void DoAction(Page* page, Widget* widget, double value) override
    {
        action_->Do(page, commandId_);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GlobalContextWithIntParam : public ActionContext
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    int param_ = 0;
    
public:
    GlobalContextWithIntParam(Action* action, int param, bool isInverted) : ActionContext(action, isInverted), param_(param) {}
    
    virtual void RequestActionUpdate(Page* page, Widget* widget) override
    {
        // GAW TBD
    }
    
    virtual void DoAction(Page* page, Widget* widget, double value) override
    {
        action_->Do(page, param_);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TrackCycleContext : public TrackContext
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    Widget* cyclerWidget_ = nullptr;
    int index = 0;
    vector<Action*> actions_;
    
public:
    TrackCycleContext(map<string, Action*>& availableActions, vector<string> params, Action* action, bool isInverted) : TrackContext(action, isInverted)
    {
        for(int i = 2; i < params.size(); i++)
            if(Action* availableAction = availableActions[params[i]])
                actions_.push_back(availableAction);
    }
    
    virtual void SetCyclerWidget(Widget* cyclerWidget) override { cyclerWidget_ = cyclerWidget; }
    
    virtual void RequestActionUpdate(Page* page, Widget* widget) override
    {
        if(MediaTrack* track = DAW::GetTrackFromGUID(trackGUID_, page->GetFollowMCP()))
        {
            if(actions_[index])
                actions_[index]->RequestUpdate(page, this, widget, track);
        }
        else
        {
            widget->SetValue(0, 0.0);
            widget->SetValue("");
        }
    }
    
    virtual void DoAction(Page* page, Widget* widget, double value) override
    {
        if(widget && widget == cyclerWidget_)
            index = index < actions_.size() - 1 ? index + 1 : 0;
        else if(actions_[index])
            if(MediaTrack* track = DAW::GetTrackFromGUID(trackGUID_, page->GetFollowMCP()))
                actions_[index]->Do(page, widget, track, value);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PageSurfaceContext : public ActionContext
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    PageSurfaceContext(Action* action, bool isInverted) : ActionContext(action, isInverted) {}
    
    virtual void DoAction(Page* page, RealSurface* surface) override
    {
            action_->Do(page, surface);
    }
    
    virtual void DoAction(Page* page, Widget* widget, double value) override
    {
        action_->Do(page, widget->GetSurface(), value);
    }
};

#endif /* control_surface_base_actions_h */
