//
//  control_surface_integrator.h
//  reaper_control_surface_integrator
//
//

#ifndef control_surface_integrator
#define control_surface_integrator

#include <sstream>
#include <vector>
#include <map>

using namespace std;

#include "control_surface_integrator_Reaper.h"

const string LogicalCSurf = "LogicalCSurf";

const string Shift = "Shift";
const string Option = "Option";
const string Control = "Control";
const string Alt = "Alt";

struct MapEntry
{
    string widgetName;
    string paramName;
    
    MapEntry(string aWidgetName, string aParamName) : widgetName(aWidgetName), paramName(aParamName) {}
};

struct FXMap
{
private:
    string name;
    vector<MapEntry> entries_;
    
public:
    FXMap(string aName) : name(aName) {}
    
    string GetName() { return name; }
    
    void AddEntry(string widgetName, string paramName) { entries_.push_back(MapEntry(widgetName, paramName));   }
    
    vector<MapEntry>& GetMapEntries() { return entries_; }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RealCSurf;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MidiWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    string GUID_ = "";
    RealCSurf* surface_ = nullptr;
    string name_= "";
    MIDI_event_ex_t* midiPressMessage_ = nullptr;
    MIDI_event_ex_t* midiReleaseMessage_ = nullptr;
    
protected:
    string GetGUID() { return GUID_; }
    RealCSurf* GetSurface() { return surface_; }
    MIDI_event_ex_t* GetMidiReleaseMessage() { return midiReleaseMessage_; }
    MIDI_event_ex_t* GetMidiPressMessage() { return midiPressMessage_; }

public:
    virtual ~MidiWidget() {};
    
    MidiWidget(string GUID, RealCSurf* surface, string name, MIDI_event_ex_t* press, MIDI_event_ex_t* release) : GUID_(GUID), surface_(surface), name_(name), midiPressMessage_(press), midiReleaseMessage_(release) {}
    
    virtual double GetMinDB() { return -72.0; }
    virtual double GetMaxDB() { return 12.0; }
    string GetName() { return name_; }
    void SetGUID(string GUID) { GUID_ = GUID; }
    virtual void ProcessMidiMessage(const MIDI_event_ex_t* midiMessage) {}
    void Update();
    void ForceUpdate();
    virtual void SetValue(double value) {}
    virtual void SetValue(double value, int displaymode) {}
    virtual void SetValue(string value) {}
    virtual void SetValueToZero() {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LogicalSurface;
class CSurfChannel;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RealCSurf
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
protected:
    const string name_ = "";
    LogicalSurface* logicalSurface_ = nullptr;
    string bankGroup_ = "";
    int numBankableChannels_ = 0;
    vector<CSurfChannel*> channels_;
    map<string, MidiWidget*> widgets_;
    
    bool shift_ = false;
    bool option_ = false;
    bool control_ = false;
    bool alt_ = false;
    
    bool zoom_ = false;
    bool scrub_ = false;
    
    string CurrentModifers()
    {
        string modifiers = "";
        
        if(shift_)
            modifiers += Shift;
        if(option_)
            modifiers += Option;
        if(control_)
            modifiers +=  Control;
        if(alt_)
            modifiers += Alt;
        
        return modifiers;
    }
    
    string ActionAddressFor(string GUID, string name)
    {
        return GUID + GetName() + CurrentModifers() + name;
    }
    
public:
    virtual ~RealCSurf() {};
    
    RealCSurf(LogicalSurface* logicalSurface, string bankGroup, const string name, int numBankableChannels) : logicalSurface_(logicalSurface), bankGroup_(bankGroup), name_(name),  numBankableChannels_(numBankableChannels) {}
    
    const string GetName() const { return name_; }

    LogicalSurface* GetLogicalSurface() { return logicalSurface_; }

    vector<CSurfChannel*> & GetChannels() { return channels_; }

    int GetNumBankableChannels() { return numBankableChannels_; }
    
    virtual void SendMidiMessage(MIDI_event_ex_t* midiMessage) {}
    
    virtual void SendMidiMessage(int first, int second, int third) {}
    
    virtual void OnTrackSelection(MediaTrack *track);
    
    virtual void RunAndUpdate() {}
    
    void AddChannel(CSurfChannel*  channel)
    {
        channels_.push_back(channel);
    }
    
    void AddWidget(MidiWidget* widget)
    {
        widgets_[widget->GetName()] = widget;
    }
    
    void SetWidgetGUID(string widgetName, string GUID) { widgets_[widgetName]->SetGUID(GUID); }
    void SetWidgetValueToZero(string widgetName) { widgets_[widgetName]->SetValueToZero(); }

    void SetShift(bool value) { shift_ = value; ForceUpdateWidgets(); }
    void SetOption(bool value) { option_ = value; ForceUpdateWidgets(); }
    void SetControl(bool value) { control_ = value; ForceUpdateWidgets(); }
    void SetAlt(bool value) { alt_ = value; ForceUpdateWidgets(); }
    void SetZoom(bool value) { zoom_ = value; ForceUpdateWidgets(); }
    void SetScrub(bool value) { scrub_ = value; ForceUpdateWidgets(); }
    
    bool IsZoom() { return zoom_; }
    bool IsScrub() { return scrub_; }

    // to Widgets ->
    virtual void UpdateWidgets()
    {
        for(auto const& [key, val] : widgets_ )
            val->Update();
    }

    virtual void ForceUpdateWidgets()
    {
        for(auto const& [key, val] : widgets_ )
            val->ForceUpdate();
    }

    // to Actions ->
    double GetCurrentNormalizedValue(string surfaceName, string widgetName);
    void UpdateAction(string surfaceName, string widgetName);
    void ForceUpdateAction(string surfaceName, string widgetName);
    void CycleAction(string surfaceName, string widgetName);
    void RunAction(string surfaceName, string widgetName, double value);
    
    // to Widgets ->
    void SetWidgetValue(string widgetName, double value) { widgets_[widgetName]->SetValue(value); }
    void SetWidgetValue(string widgetName, double value, int mode) { widgets_[widgetName]->SetValue(value, mode); }
    void SetWidgetValue(string widgetName, string value) { widgets_[widgetName]->SetValue(value); }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSurfChannel
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    string GUID_ = "";
    RealCSurf* surface_= nullptr;
    bool isMovable_ = true;
    vector<string> widgetNames_;

public:
    virtual ~CSurfChannel() {}
    
    CSurfChannel(string GUID, RealCSurf* surface, bool isMovable) : GUID_(GUID), surface_(surface), isMovable_(isMovable){}
    
    string GetGUID() { return GUID_; }
    
    RealCSurf* GetSurface() { return surface_; }
    
    bool GetIsMovable() { return isMovable_; }
    
    void OnTrackSelection(MediaTrack *trackid);
    void MapFX(MediaTrack *trackid);
    
    void SetIsMovable(bool isMovable) { isMovable_ = isMovable; }
    
    void AddWidget(MidiWidget* widget)
    {
        widgetNames_.push_back(widget->GetName());
        surface_->AddWidget(widget);
    }
    
    void SetGUID(string GUID)
    {
        GUID_ = GUID;
        
        for (auto widgetName : widgetNames_)
        {
            surface_->SetWidgetGUID(widgetName, GUID);

            if(GUID_ == "")
                surface_->SetWidgetValueToZero(widgetName);
         }
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
protected:
    LogicalSurface* logicalSurface_ = nullptr;
    
    Action(LogicalSurface* logicalSurface) : logicalSurface_(logicalSurface) {}
    
    LogicalSurface* GetLogicalSurface() { return logicalSurface_; }
    
    virtual void SetWidgetValue(string surfaceName, string widgetName, double value) {}
    virtual void SetWidgetValue(string surfaceName, string widgetName, string value) {}

public:
    virtual ~Action() {}
    
    virtual int GetDisplayMode() { return 0; }
    
    virtual double GetCurrentNormalizedValue () { return 0.0; }

    virtual void Add(Action* action) {}
    
    virtual void Update(string surfaceName, string widgetName) {}
    virtual void ForceUpdate(string surfaceName, string widgetName) {}
    virtual void Cycle(string surfaceName, string widgetName) {}
    virtual void Run(double value, string surfaceName, string widgetName) {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSurfManager;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LogicalSurface
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    CSurfManager* manager_ = nullptr;
    map<string, FXMap *> fxMaps_;
    vector<RealCSurf*> surfaces_;
    map<string, Action*> actions_;
    
    int numLogicalChannels_ = 0;
    int trackOffset_ = 0;
    bool VSTMonitor_ = false;

    void BuildTrackActions();
    void BuildTrackActions2();
    void BuildCSurfWidgets();

    int GetNumLogicalChannels() { return numLogicalChannels_; }
    
    bool DidTrackListChange();
    
    void AddFXMap(FXMap* fxMap)
    {
        fxMaps_[fxMap->GetName()] = fxMap;
    }
    
    void AddSurface(RealCSurf* surface)
    {
        surfaces_.push_back(surface);
    }
 
    void AddAction(string actionAddress, Action* action)
    {
        actions_[actionAddress] = action;
    }
    
    void RebuildTrackInteractors()
    {
        //trackInteractors_.clear();
        BuildTrackActions();
        RefreshLayout();
    }
    
public:
    LogicalSurface(CSurfManager* manager) : manager_(manager) {}

    CSurfManager* GetManager() { return manager_; }
    
    map<string, FXMap *> GetFXMaps() { return fxMaps_; }
    
    bool GetVSTMonitor() { return VSTMonitor_; }

    RealCSurf* GetSurface(string name)
    {
        for(auto* surface : surfaces_)
            if(surface->GetName() == name)
                return surface;
        
        return nullptr;
    }
    
    void OnTrackSelection(MediaTrack* track)
    {
        for(auto& surface : surfaces_)
            surface->OnTrackSelection(track);
    }

    void RunAndUpdate()
    {
        for(auto & surface : surfaces_)
            surface->RunAndUpdate();
    }

    void TrackListChanged()
    {
        if(DidTrackListChange())
            RebuildTrackInteractors();
    }

    void Initialize();
    void Initialize2();
    void InitializeFXMaps();
    void InitializeSurfaces();
    void InitializeLogicalCSurfInteractor();
    
    void RefreshLayout();

    void SetShift(string surfaceName, bool value);
    void SetOption(string surfaceName, bool value);
    void SetControl(string surfaceName, bool value);
    void SetAlt(string surfaceName, bool value);
    
    void SetZoom(string surfaceName, bool value);
    void SetScrub(string surfaceName, bool value);
    
    void AdjustTrackBank(int stride);
    void ImmobilizeSelectedTracks();
    void MobilizeSelectedTracks();
    
    void TrackFXListChanged(MediaTrack* track);
    void MapFX(MediaTrack* track);

    // to Widgets ->
    void ForceUpdate();
    
    // to Actions ->
    double GetCurrentNormalizedValue(string actionAddress, string surfaceName, string widgetName);

    void UpdateAction(string actionAddress, string surfaceName, string widgetName);
    void ForceUpdateAction(string actionAddress, string surfaceName, string widgetName);
    void CycleAction(string actionAddress, string surfaceName, string widgetName);
    void RunAction(string actionAddress, double value, string surfaceName, string widgetName);
    
    // to Widgets ->
    void SetWidgetValue(string surfaceName, string widgetName, double value);
    void SetWidgetValue(string surfaceName, string widgetName, double value, int mode);
    void SetWidgetValue(string surfaceName, string widgetName, string value);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MidiIOManager
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct MidiChannelInput // inner struct
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        int channel_ = 0;
        midi_Input* midiInput_ = nullptr;
        
        MidiChannelInput(int channel, midi_Input* midiInput)
        : channel_(channel), midiInput_(midiInput) {}
    };
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct MidiChannelOutput // inner struct
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        int channel_ = 0;
        midi_Output* midiOutput_ = nullptr;
        
        MidiChannelOutput(int channel, midi_Output* midiOutput)
        : channel_(channel), midiOutput_(midiOutput) {}
    };
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // class MidiIOManager starts here
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
    vector<MidiChannelInput> inputs_;
    vector<MidiChannelOutput> outputs_;
    
public:
    MidiIOManager() {}
    
    midi_Input* GetMidiInputForChannel(int inputChannel)
    {
        for(auto input : inputs_)
            if(input.channel_ == inputChannel)
                return input.midiInput_; // return existing
        
        // make new
        midi_Input* newInput = DAW::CreateMIDIInput(inputChannel);
        
        if(newInput)
        {
            newInput->start();
            inputs_.push_back(MidiChannelInput(inputChannel, newInput));
            return newInput;
        }
        
        return nullptr;
    }
    
    midi_Output* GetMidiOutputForChannel(int outputChannel)
    {
        for(auto output : outputs_)
            if(output.channel_ == outputChannel)
                return output.midiOutput_; // return existing
        
        // make new
        midi_Output* newOutput = DAW::CreateMIDIOutput(outputChannel, false, NULL );
        
        if(newOutput)
        {
            outputs_.push_back(MidiChannelOutput(outputChannel, newOutput));
            return newOutput;
        }
        
        return nullptr;
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSurfManager
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    MidiIOManager* midiIOManager_ = nullptr;
    
    vector <LogicalSurface*> logicalSurfaces_;

    bool lazyInitialized_ = false;
    
    int currentLogicalSurfaceIndex_ = 0;;

    void RunAndUpdate()
    {
        if(!lazyInitialized_)
        {
            // init the maps here
            
            LogicalSurface* logicalSurface = new LogicalSurface(this);
            logicalSurface->Initialize();
            logicalSurfaces_.push_back(logicalSurface);
            
            logicalSurface = new LogicalSurface(this);
            logicalSurface->Initialize2();
            logicalSurfaces_.push_back(logicalSurface);
            
            lazyInitialized_ = true;
        }
        
        logicalSurfaces_[currentLogicalSurfaceIndex_]->RunAndUpdate();
    }
    
    double GetPrivateProfileDouble(string key)
    {
        char tmp[512];
        memset(tmp, 0, sizeof(tmp));
        
        DAW::GetPrivateProfileString("REAPER", key.c_str() , "", tmp, sizeof(tmp), get_ini_file());

        return strtod (tmp, NULL);
    }

    
public:
    virtual ~CSurfManager() {};
    
    CSurfManager() { midiIOManager_ = new MidiIOManager(); }
    
    MidiIOManager* MidiManager() { return midiIOManager_; }
    
    bool GetLazyInitialized() { return lazyInitialized_; }
    
    double GetFaderMaxDB()
    {
        return GetPrivateProfileDouble("slidermaxv");
    }
    
    double GetFaderMinDB()
    {
        return GetPrivateProfileDouble("sliderminv");
    }
    
    double GetVUMaxDB()
    {
        return GetPrivateProfileDouble("vumaxvol");
    }
    
    double GetVUMinDB()
    {
        return GetPrivateProfileDouble("vuminvol");
    }
    
    void OnTrackSelection(MediaTrack *trackid)
    {
        logicalSurfaces_[currentLogicalSurfaceIndex_]->OnTrackSelection(trackid);
    }
    
    void Run()
    {
        RunAndUpdate();
    }
    
    void NextLogicalSurface()
    {
        currentLogicalSurfaceIndex_ = currentLogicalSurfaceIndex_ == logicalSurfaces_.size() - 1 ? 0 : ++currentLogicalSurfaceIndex_;

        logicalSurfaces_[currentLogicalSurfaceIndex_]->RefreshLayout();
    }

    void TrackListChanged() // tell current map
    {
        if(logicalSurfaces_.size() != 0) // seems we need to protect against prematurely early calls
            for(auto & surface : logicalSurfaces_)
                surface->TrackListChanged();
    }

    void TrackFXListChanged(MediaTrack* trackid)
    {
        if(logicalSurfaces_.size() != 0) // seems we need to protect against prematurely early calls
            for(auto & surface : logicalSurfaces_)
                surface->TrackFXListChanged(trackid);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class OSCCSurf : public RealCSurf
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    virtual ~OSCCSurf() {};
    
    OSCCSurf(const string name, LogicalSurface* surface)
    : RealCSurf(surface, "", "OSC", 8) {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class WebCSurf : public RealCSurf
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    virtual ~WebCSurf() {};
    
    WebCSurf(const string name, LogicalSurface* surface)
    : RealCSurf(surface, "", "Web", 8) {};
};

#endif /* control_surface_integrator.h */
