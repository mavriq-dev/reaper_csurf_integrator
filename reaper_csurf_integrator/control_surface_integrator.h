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

const string Volume = "Volume";
const string Pan = "Pan";

const string Shift = "Shift";
const string Option = "Option";
const string Control = "Control";
const string Alt = "Alt";

const string Delimiter = "#";

template <class Container>
static void SplitString(string& str, Container& cont, char delim = ' ')
{
    stringstream ss(str);
    string token;
    while (getline(ss, token, delim))
        cont.push_back(token);
};

struct MapEntry
{
    string widget;
    string param;
    
    MapEntry(string aWidget, string aParam) : widget(aWidget), param(aParam) {}
};

struct FXMap
{
private:
    string name_;
    vector<MapEntry> entries_;
    
public:
    FXMap(string name) : name_(name) {}
    
    string GetName() { return name_; }
    
    void AddEntry(string widget, string param) { entries_.push_back(MapEntry(widget, param));   }
    
    vector<MapEntry>& GetMapEntries() { return entries_; }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSurfManager;
class LogicalSurface;
class Interactor;
class Action;
class RealCSurf;
class CSurfChannel;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MidiWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    string name_= "";
    CSurfChannel* channel_ = nullptr;
    MIDI_event_ex_t* midiPressMessage_ = nullptr;
    MIDI_event_ex_t* midiReleaseMessage_ = nullptr;
    
protected:
    MIDI_event_ex_t* GetMidiReleaseMessage() { return midiReleaseMessage_; }
    MIDI_event_ex_t* GetMidiPressMessage() { return midiPressMessage_; }
    
public:
    virtual ~MidiWidget() {};
    
    MidiWidget(string name, CSurfChannel* channel, MIDI_event_ex_t* press, MIDI_event_ex_t* release) : name_(name), channel_(channel), midiPressMessage_(press), midiReleaseMessage_(release) {}
    
    CSurfChannel* GetChannel() { return channel_; }

    virtual double GetMinDB() { return 0.0; }
    
    virtual double GetMaxDB() { return 1.0; }
    
    string GetName();

    virtual void ProcessMidiMessage(const MIDI_event_ex_t* midiMessage) {}

    void Update();
    void ForceUpdate();
    virtual void SetValue(double value) {}
    virtual void SetValue(double value, int displaymode) {}
    virtual void SetValue(string value) {}
    virtual void SetValueToZero() {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SubChannel
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    string subGUID_ = "";
    vector<string> widgetNames_;
    
public:
    SubChannel(string subGUID) : subGUID_(subGUID) {}
    
    string GetSubGUID() { return subGUID_; }
    
    vector<string> GetWidgetNames() { return widgetNames_; }
    
    void AddWidgetName(string name)
    {
        widgetNames_.push_back(name);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSurfChannel
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    string GUID_ = "";
    RealCSurf* surface_= nullptr;
    vector<MidiWidget*> widgets_;
    int shouldMapSubChannels_ = 0;
    vector<SubChannel*> subChannels_;
    
protected:
    void MapFX(MediaTrack *trackid);

public:
    virtual ~CSurfChannel() {}
    
    CSurfChannel(string GUID, RealCSurf* surface) : GUID_(GUID), surface_(surface) {}
    
    CSurfChannel(string GUID, RealCSurf* surface, int shouldMapSubChannels) : GUID_(GUID), surface_(surface), shouldMapSubChannels_(shouldMapSubChannels) {}
    
    RealCSurf* GetSurface() { return surface_; }
    
    string GetGUID() { return GUID_; }
    
    vector<MidiWidget*> GetWidgets() { return widgets_; }
    
    vector<SubChannel*> GetSubChannels() { return subChannels_; }

    virtual void OnTrackSelection(MediaTrack *trackid);
    
    void AddWidget(MidiWidget* widget)
    {
        widgets_.push_back(widget);
    }
    
    void AddSubChannel(SubChannel* subChannel)
    {
        subChannels_.push_back(subChannel);
    }

    void SetGUID(string GUID)
    {
        GUID_ = GUID;
    }
    
    void ProcessMidiMessage(const MIDI_event_ex_t* evt);
    
    virtual void Update();
    virtual void ForceUpdate();
    
    void RunAction(string name, double value);
    void CycleAction(string name);
    
    virtual void SetWidgetValue(string name, double value);
    virtual void SetWidgetValue(string name, double value, int mode);
    virtual void SetWidgetValue(string name, string value);
    
    void SetWidgetValue(string subGUID, string name, double value);
    void SetWidgetValue(string subGUID, string name, double value, int mode);
    void SetWidgetValue(string subGUID, string name, string value);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RealCSurf
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    const string name_ = "";
    LogicalSurface* logicalSurface_ = nullptr;
    const int numChannels_ = 0;
    vector<CSurfChannel*> channels_;

public:
    virtual ~RealCSurf() {};
    
    RealCSurf(const string name, LogicalSurface* logicalSurface,  const int numChannels) : name_(name), logicalSurface_(logicalSurface),  numChannels_(numChannels) {}
    
    LogicalSurface* GetLogicalSurface() { return logicalSurface_; }
    
    vector<CSurfChannel*> & GetChannels() { return channels_; }
    
    const string GetName() const { return name_; }
    
    const int GetNumChannels() const { return numChannels_; }

    virtual void SendMidiMessage(MIDI_event_ex_t* midiMessage) {}
    
    virtual void SendMidiMessage(int first, int second, int third) {}

    virtual void OnTrackSelection(MediaTrack *trackid);
    
    virtual void RunAndUpdate() {}
    
    virtual void AddChannel(CSurfChannel*  channel)
    {
        channels_.push_back(channel);
    }
    
    virtual void Update();
    virtual void ForceUpdate();
    
    void SetWidgetValue(string GUID, string name, double value);
    void SetWidgetValue(string GUID, string name, double value, int mode);
    void SetWidgetValue(string GUID, string name, string value);
    
    void SetWidgetValue(string GUID, string subGUID, string name, double value);
    void SetWidgetValue(string GUID, string subGUID, string name, double value, int mode);
    void SetWidgetValue(string GUID, string subGUID, string name, string value);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Action
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    string name_ = "";
    Interactor* interactor_ = nullptr;

protected:
    virtual void SetWidgetValue(double value) {}
    virtual void SetWidgetValue(string value) {}

    Action(string name, Interactor* interactor) : name_(name), interactor_(interactor) {}
    
public:
    virtual ~Action() {}

    Interactor* GetInteractor() { return interactor_; }
    
    string GetName() { return name_; }
    
    virtual string GetAlias() { return name_; }
    
    virtual int GetDisplayMode() { return 0; }
    
    virtual double GetCurrentNormalizedValue () { return 0.0; }

    virtual void AddAction(Action* action) {}
    
    virtual void Update() {}
    virtual void ForceUpdate() {}
    virtual void Cycle() {}
    virtual void RunAction(double value) {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SubInteractor;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Interactor
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
protected:
    string GUID_ = "";
    LogicalSurface* logicalSurface_ = nullptr;
    map <string, vector<Action*>> actions_;
    vector<SubInteractor*> subInteractors_;
    
public:
    virtual ~Interactor() {}
    
    Interactor(string GUID, LogicalSurface* logicalSurface) : GUID_(GUID), logicalSurface_(logicalSurface) {}

    virtual string GetGUID() { return GUID_; }

    virtual LogicalSurface* GetLogicalSurface() { return logicalSurface_; }
    
    virtual MediaTrack* GetTrack() { return DAW::GetTrackFromGUID(GetGUID()); }
    
    virtual int GetIndex() { return 0; }
    
    double GetCurrentNormalizedValue(string name)
    {
        if(actions_[name].size() > 0)
            return (actions_[name])[0]->GetCurrentNormalizedValue();
        else
            return 0.0;
    }
    
    void AddAction(Action* action)
    {
        actions_[action->GetName()].push_back(action);
    }
    
    void AddAliasedAction(Action* action)
    {
        actions_[action->GetAlias()].push_back(action);
    }
    
    void AddSubInteractor(SubInteractor* subInteractor)
    {
        subInteractors_.push_back(subInteractor);
    }
    
    void Update(string name);
    void ForceUpdate(string name);
    void RunAction(string name, double value);
    void CycleAction(string name);
    
    virtual void SetWidgetValue(string name, double value);
    virtual void SetWidgetValue(string name, double value, int mode);
    virtual void SetWidgetValue(string name, string value);
    
    void SetWidgetValue(string SubGUID, string name, double value);
    void SetWidgetValue(string SubGUID, string name, double value, int mode);
    void SetWidgetValue(string SubGUID, string name, string value);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SubInteractor : public Interactor
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    string subGUID_ = "";
    int index_ = 0;
    Interactor* interactor_ = nullptr;
    
    Interactor* GetInteractor() { return interactor_; }
    
public:
    virtual ~SubInteractor() {}

    SubInteractor(string subGUID, int index, Interactor* interactor) : Interactor(interactor->GetGUID(), interactor->GetLogicalSurface()), subGUID_(subGUID), index_(index), interactor_(interactor) {}
    
    virtual string GetGUID() override { return subGUID_; }
    
    virtual LogicalSurface* GetLogicalSurface() override { return interactor_->GetLogicalSurface(); }
    
    virtual MediaTrack* GetTrack() override { return DAW::GetTrackFromGUID(interactor_->GetGUID()); }
    
    virtual int GetIndex() override { return index_; }
    
    virtual void SetWidgetValue(string name, double value) override;
    virtual void SetWidgetValue(string name, double value, int mode) override;
    virtual void SetWidgetValue(string name, string value) override;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LogicalSurface
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    CSurfManager* manager_ = nullptr;
    map<string, FXMap *> fxMaps_;
    vector<RealCSurf*> surfaces_;
    vector<Interactor*> interactors_;
    vector<string> immovableTrackGUIDs_;
    bool isSynchronized_ = false;
    bool isFlipped_ = false;
    int trackOffset_ = 0;
    
    bool shift_ = false;
    bool option_ = false;
    bool control_ = false;
    bool alt_ = false;
    
    bool zoom_ = false;
    bool scrub_ = false;

    void BuildTrackInteractors();
    void BuildTrackInteractors2();
    void BuildCSurfWidgets();

    // There is an immovable GUID slot for each channel, so by definition immovableTrackGUIDs_.size is number of channels
    int NumChannels() { return (int)immovableTrackGUIDs_.size(); }
    
    bool DidTrackListChange();

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
    
    string FlipNameFor(string name)
    {
        if(name == Volume && isFlipped_)
            return Pan;
        else if(name == Pan && isFlipped_)
            return Volume;
        else
            return name;
    }
    
    string ModifiedNameFor(string name)
    {
        if(name == Shift || name == Option || name == Control || name == Alt)
            return name;
        
        if(CurrentModifers() != "")
            return CurrentModifers() + "#" + name;
        else return name;
    }
    
    string UnmodifiedNameFor(string name)
    {
        vector<string> tokens;
        
        SplitString(name, tokens, '#');
        
        if(tokens.size() == 1)
            return name;
        else
            return tokens[1];
    }
    
    bool IsOKToSetWidget(string name)
    {
        vector<string> tokens;
        
        SplitString(name, tokens, '#');
        
        if((tokens.size() == 1 && CurrentModifers() == "") || (tokens.size() > 0 && tokens[0] == CurrentModifers()))
            return true;
        else
            return false;
    }
    
    CSurfChannel* Channel(int wantChannel)
    {
        int offset = 0;
        
        for(auto & surface : surfaces_)
        {
            if(surface->GetNumChannels() + offset - 1  < wantChannel)
                offset += surface->GetNumChannels();
            else
                return surface->GetChannels()[wantChannel - offset];
        }
        
        return nullptr;
    }
    
    bool isInImmovableTrackGUIDS(string trackGUID)
    {
        for(auto & immovableTrackGUID : immovableTrackGUIDs_)
            if(immovableTrackGUID == trackGUID)
                return true;
        
        return false;
    }
    
    void SetSynchronized(bool isSynchronized)
    {
        isSynchronized_ = isSynchronized;
    }
    
    void AddFXMap(FXMap* fxMap)
    {
        fxMaps_[fxMap->GetName()] = fxMap;
    }
    
    void AddInteractor(Interactor* interactor)
    {
        interactors_.push_back(interactor);
    }
    
    void AddSurface(RealCSurf* surface)
    {
        surfaces_.push_back(surface);
    }
    
    void RebuildInteractors()
    {
        interactors_.clear();
        BuildTrackInteractors();
        RefreshLayout();
    }
    
public:
    LogicalSurface(CSurfManager* manager) : manager_(manager) {}

    CSurfManager* GetManager() { return manager_; }
    
    map<string, FXMap *> GetFXMaps() { return fxMaps_; }
    
    bool IsZoom() { return zoom_; }
    
    bool IsScrub() { return scrub_; }
    
    bool IsFlipped() { return isFlipped_; }
    
    void OnTrackSelection(MediaTrack* trackid)
    {
        for(auto& surface : surfaces_)
            surface->OnTrackSelection(trackid);
    }

    void RunAndUpdate()
    {
        for(auto & surface : surfaces_)
            surface->RunAndUpdate();
    }

    void TrackListChanged()
    {
        if(DidTrackListChange())
            RebuildInteractors();
    }
    
    void ToggleFlipped(string name)
    {
        isFlipped_ = ! isFlipped_;
        
        SetWidgetValue("", name, isFlipped_);
    }

    void Initialize();
    void Initialize2();
    void InitializeFXMaps();
    void InitializeSurfaces();
    void InitializeLogicalCSurfInteractors();
    
    double GetCurrentNormalizedValue(string trackGUID, string name);
    
    void SetShift(bool value) { shift_ = value; ForceUpdate(); }
    void SetOption(bool value) { option_ = value; ForceUpdate(); }
    void SetControl(bool value) { control_ = value; ForceUpdate(); }
    void SetAlt(bool value) { alt_ = value; ForceUpdate(); }
    void SetZoom(bool value) { zoom_ = value; ForceUpdate(); }
    void SetScrub(bool value) { scrub_ = value; ForceUpdate(); }

    void RefreshLayout();
    void ForceUpdate();

    void Update(string GUID, string name);
    void ForceUpdate(string GUID, string name);
    void CycleAction(string trackGUID, string name);
    void RunAction(string GUID, string name, double value);

    void SetWidgetValue(string GUID, string name, double value);
    void SetWidgetValue(string GUID, string name, double value, int mode);
    void SetWidgetValue(string GUID, string name, string value);
    
    void SetWidgetValue(string GUID, string subGUID, string name, double value);
    void SetWidgetValue(string GUID, string subGUID, string name, double value, int mode);
    void SetWidgetValue(string GUID, string subGUID, string name, string value);
    
    void AdjustTrackBank(int stride);
    void ImmobilizeSelectedTracks();
    void MobilizeSelectedTracks();
    
    void TrackFXListChanged(MediaTrack* trackid);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MidiIOManager;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSurfManager
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    MidiIOManager* midiIOManager_ = nullptr;
    
    vector <LogicalSurface*> surfaces_;

    bool lazyInitialized_ = false;
    
    int currentSurfaceIndex_ = 0;;

    void RunAndUpdate()
    {
        if(!lazyInitialized_)
        {
            // init the maps here
            
            LogicalSurface* surface = new LogicalSurface(this);
            surface->Initialize();
            surfaces_.push_back(surface);
            
            surface = new LogicalSurface(this);
            surface->Initialize2();
            surfaces_.push_back(surface);
            
            lazyInitialized_ = true;
        }
        
        surfaces_[currentSurfaceIndex_]->RunAndUpdate();
    }
    
    double GetPrivateProfileDouble(string key)
    {
        char tmp[512];
        memset(tmp, 0, sizeof(tmp));
        
        DAW::GetPrivateProfileString("REAPER", key.c_str() , "", tmp, sizeof(tmp), get_ini_file());

        return strtod (tmp, NULL);
    }

    
public:
    virtual ~CSurfManager() { };
    
    CSurfManager();
    
    MidiIOManager* MidiManager() { return midiIOManager_; }
    
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
        surfaces_[currentSurfaceIndex_]->OnTrackSelection(trackid);
    }
    
    void Run()
    {
        RunAndUpdate();
    }
    
    void NextLogicalSurface()
    {
        currentSurfaceIndex_ = currentSurfaceIndex_ == surfaces_.size() - 1 ? 0 : ++currentSurfaceIndex_;

        surfaces_[currentSurfaceIndex_]->RefreshLayout();
    }
    
    void ImmobilizeSelectedTracks(LogicalSurface* logicalCSurf)
    {
        for(auto & surface : surfaces_)
            if(surface != logicalCSurf)
                surface->ImmobilizeSelectedTracks();
    }
    
   void  MobilizeSelectedTracks(LogicalSurface* logicalCSurf)
    {
        for(auto & surface : surfaces_)
            if(surface != logicalCSurf)
                surface->MobilizeSelectedTracks();
    }
    
    void AdjustTrackBank(LogicalSurface* logicalCSurf, int stride)
    {
        for(auto & surface : surfaces_)
            if(surface != logicalCSurf)
                surface->AdjustTrackBank(stride);
    }

    void TrackListChanged() // tell current map
    {
        if(surfaces_.size() != 0) // seems we need to protect against prematurely early calls
            for(auto & surface : surfaces_)
                surface->TrackListChanged();
    }
    
    void TrackFXListChanged(MediaTrack* trackid)
    {
        if(surfaces_.size() != 0) // seems we need to protect against prematurely early calls
            for(auto & surface : surfaces_)
                surface->TrackFXListChanged(trackid);
    }
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
    CSurfManager* manager_ = nullptr;
    vector<MidiChannelInput> inputs_;
    vector<MidiChannelOutput> outputs_;
    
public:
    MidiIOManager(CSurfManager* manager) : manager_(manager) {}
    
    CSurfManager* GetManager() { return  manager_; }
    
    midi_Input* GetMidiInputForChannel(int inputChannel)
    {
        for(auto input : inputs_)
            if(input.channel_ == inputChannel)
                return input.midiInput_;
        
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
                return output.midiOutput_;
        
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
class OSCCSurf : public RealCSurf
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    virtual ~OSCCSurf() {};
    
    OSCCSurf(LogicalSurface* surface, const string name, const int numChannels)
    : RealCSurf(name, surface, numChannels) {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class WebCSurf : public RealCSurf
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    virtual ~WebCSurf() {};
    
    WebCSurf(LogicalSurface* surface, const string name, const int numChannels)
    : RealCSurf(name, surface, numChannels) {};
};

#endif /* control_surface_integrator.h */
