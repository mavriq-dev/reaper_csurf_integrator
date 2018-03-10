//
//  control_surface_manager_actions.h
//  reaper_csurf_integrator
//
//

#ifndef control_surface_manager_actions_h
#define control_surface_manager_actions_h

#include "control_surface_base_actions.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Shift_Action : public Double_Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    Shift_Action(Layer* layer) : Double_Action(layer)  {}
    
    virtual void Do(double value, string zoneName, string surfaceName, string widgetName) override
    {
        GetLayer()->SetShift(zoneName, surfaceName, value);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Option_Action : public Double_Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    Option_Action(Layer* layer) : Double_Action(layer)  {}
    
    virtual void Do(double value, string zoneName, string surfaceName, string widgetName) override
    {
        GetLayer()->SetOption(zoneName, surfaceName, value);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Control_Action : public Double_Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    Control_Action(Layer* layer) : Double_Action(layer)  {}
    
    virtual void Do(double value, string zoneName, string surfaceName, string widgetName) override
    {
        GetLayer()->SetControl(zoneName, surfaceName, value);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Alt_Action : public Double_Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    Alt_Action(Layer* layer) : Double_Action(layer)  {}
    
    virtual void Do(double value, string zoneName, string surfaceName, string widgetName) override
    {
        GetLayer()->SetAlt(zoneName, surfaceName, value);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Latched_Action : public Double_Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    clock_t lastPressed_ = clock();
public:
    Latched_Action(Layer* layer) : Double_Action(layer)  {}
    
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
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LatchedZoom_Action : public Latched_Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    LatchedZoom_Action(Layer* layer) : Latched_Action(layer)  {}
    
    virtual void SetValue(string zoneName, string surfaceName, double value) override
    {
        GetLayer()->SetZoom(zoneName, surfaceName, value);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LatchedScrub_Action : public Latched_Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    LatchedScrub_Action(Layer* layer) : Latched_Action(layer)  {}
    
    virtual void SetValue(string zoneName, string surfaceName, double value) override
    {
        GetLayer()->SetScrub(zoneName, surfaceName, value);
    }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SetShowFXWindows_Action : public Double_Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    SetShowFXWindows_Action(Layer* layer) : Double_Action(layer)  {}
    
    virtual double GetValue (string zoneName, string surfaceName, string widgetName) override { return GetLayer()->IsShowFXWindows(zoneName, surfaceName); }
    
    virtual void Do(double value, string zoneName, string surfaceName, string widgetName) override
    {
        GetLayer()->SetShowFXWindows(zoneName, surfaceName, value);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class NextLayer_Action : public Double_Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    NextLayer_Action(Layer* layer) : Double_Action(layer)  {}
    
    virtual void Do(double value, string zoneName, string surfaceName, string widgetName) override
    {
        GetLayer()->GetManager()->NextLayer();
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TrackBank_Action : public Double_Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    int stride_ = 0;

public:
    TrackBank_Action(Layer* layer, string paramStr) : Double_Action(layer)
    {
        stride_ =  atol(paramStr.c_str());
    }
    
    virtual void Do(double value, string zoneName, string surfaceName, string widgetName) override
    {
        GetLayer()->AdjustTrackBank(zoneName, surfaceName, stride_);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PinSelectedTracks_Action : public Double_Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    PinSelectedTracks_Action(Layer* layer) : Double_Action(layer)  {}
    
    virtual void Do(double value, string zoneName, string surfaceName, string widgetName) override
    {
       GetLayer()->PinSelectedTracks();
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UnpinSelectedTracks_Action : public Double_Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    UnpinSelectedTracks_Action(Layer* layer) : Double_Action(layer)  {}
    
    virtual void Do(double value, string zoneName, string surfaceName, string widgetName) override
    {
        GetLayer()->UnpinSelectedTracks();
    }
};

#endif /* control_surface_manager_actions_h */
