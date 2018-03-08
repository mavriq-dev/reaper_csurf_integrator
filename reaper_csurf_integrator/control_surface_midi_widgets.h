//
//  control_surface_midi_widgets.h
//  reaper_csurf_integrator
//
//

#ifndef control_surface_midi_widgets_h
#define control_surface_midi_widgets_h

#include "control_surface_integrator.h"
#include "handy_reaper_functions.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PushButton_MidiWidget : public MidiWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
protected:
    int reverseSense_ = 0;
    
public:
    PushButton_MidiWidget(RealSurface* surface, string name, MIDI_event_ex_t* press, MIDI_event_ex_t* release) : MidiWidget(surface, name, press, release)  {}
    
    PushButton_MidiWidget(RealSurface* surface, string name, int reverseSense, MIDI_event_ex_t* press, MIDI_event_ex_t* release) : MidiWidget(surface, name, press, release), reverseSense_(reverseSense) {}
    
    void SetValue(double value) override
    {
        GetRealSurface()->SendMidiMessage(value == reverseSense_ ? GetMidiReleaseMessage() : GetMidiPressMessage());
    }
    
    virtual void SetValueToZero() override
    {
        SetValue(reverseSense_ ? 1 : 0);
    }
    
    void AddToRealSurface(RealSurface* surface) override
    {
        MidiWidget::AddToRealSurface(surface);
        surface->AddWidgetToMessageMap(to_string(GetMidiPressMessage()->midi_message[0]) + to_string(GetMidiPressMessage()->midi_message[1]) + to_string(GetMidiPressMessage()->midi_message[2]), this);
    }
    
    virtual void ProcessMidiMessage(const MIDI_event_ex_t* midiMessage) override
    {
        if(GetMidiPressMessage()->IsEqualTo(midiMessage))
            GetRealSurface()->DoAction(GetGUID(), GetActionName(), GetName(), reverseSense_ ? 0 : 1);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PushButtonWithLatch_MidiWidget : public MidiWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
protected:
    int reverseSense_ = 0;
    
public:
    PushButtonWithLatch_MidiWidget(RealSurface* surface, string name, MIDI_event_ex_t* press, MIDI_event_ex_t* release) : MidiWidget(surface, name, press, release)  {}
    
    PushButtonWithLatch_MidiWidget(RealSurface* surface, string name, int reverseSense, MIDI_event_ex_t* press, MIDI_event_ex_t* release) : MidiWidget(surface, name, press, release), reverseSense_(reverseSense) {}
    
    void SetValue(double value) override
    {
        GetRealSurface()->SendMidiMessage(value == reverseSense_ ? GetMidiReleaseMessage() : GetMidiPressMessage());
    }
    
    virtual void SetValueToZero() override
    {
        SetValue(reverseSense_ ? 1 : 0);
    }
    
    void AddToRealSurface(RealSurface* surface) override
    {
        MidiWidget::AddToRealSurface(surface);
        surface->AddWidgetToMessageMap(to_string(GetMidiPressMessage()->midi_message[0]) + to_string(GetMidiPressMessage()->midi_message[1]) + to_string(GetMidiPressMessage()->midi_message[2]), this);
        surface->AddWidgetToMessageMap(to_string(GetMidiReleaseMessage()->midi_message[0]) + to_string(GetMidiReleaseMessage()->midi_message[1]) + to_string(GetMidiReleaseMessage()->midi_message[2]), this);
    }
    
    virtual void ProcessMidiMessage(const MIDI_event_ex_t* midiMessage) override
    {
        GetRealSurface()->DoAction(GetGUID(), GetActionName(), GetName(), reverseSense_ ? 0 : 1);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PushButtonWithRelease_MidiWidget : public PushButton_MidiWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    PushButtonWithRelease_MidiWidget(RealSurface* surface, string name, MIDI_event_ex_t* press, MIDI_event_ex_t* release) : PushButton_MidiWidget(surface, name, press, release)  {}
    
    PushButtonWithRelease_MidiWidget(RealSurface* surface, string name, int reverseSense, MIDI_event_ex_t* press, MIDI_event_ex_t* release) : PushButton_MidiWidget(surface, name, reverseSense, press, release) {}
    
    void AddToRealSurface(RealSurface* surface) override
    {
        MidiWidget::AddToRealSurface(surface);
        surface->AddWidgetToMessageMap(to_string(GetMidiPressMessage()->midi_message[0]) + to_string(GetMidiPressMessage()->midi_message[1]) + to_string(GetMidiPressMessage()->midi_message[2]), this);
        surface->AddWidgetToMessageMap(to_string(GetMidiReleaseMessage()->midi_message[0]) + to_string(GetMidiReleaseMessage()->midi_message[1]) + to_string(GetMidiReleaseMessage()->midi_message[2]), this);
    }
    
    virtual void ProcessMidiMessage(const MIDI_event_ex_t* midiMessage) override
    {
        if(GetMidiPressMessage()->IsEqualTo(midiMessage))
            GetRealSurface()->DoAction(GetGUID(), GetActionName(), GetName(), reverseSense_ ? 0 : 1);
        else if(GetMidiReleaseMessage()->IsEqualTo(midiMessage))
            GetRealSurface()->DoAction(GetGUID(), GetActionName(), GetName(), reverseSense_ ? 1 : 0);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PushButtonCycler_MidiWidget : public PushButton_MidiWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    PushButtonCycler_MidiWidget(RealSurface* surface, string name, MIDI_event_ex_t* press, MIDI_event_ex_t* release) : PushButton_MidiWidget(surface, name, press, release)  {}
    
    virtual void ProcessMidiMessage(const MIDI_event_ex_t* midiMessage) override
    {
        if(GetMidiPressMessage()->IsEqualTo(midiMessage))
            GetRealSurface()->CycleAction(GetGUID(), GetActionName(), GetName());
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Fader14Bit_MidiWidget : public MidiWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    double minDB_ = 0.0;
    double maxDB_ = 0.0;
    
public:
    Fader14Bit_MidiWidget(RealSurface* surface, string name, double minDB, double maxDB, MIDI_event_ex_t* press, MIDI_event_ex_t* release) : MidiWidget(surface, name, press, release), minDB_(minDB), maxDB_(maxDB) {}
    
    double GetMinDB() override { return minDB_; }
    double GetMaxDB() override { return maxDB_; }

    virtual void SetValue(double volume) override
    {
        int volint = volume * 16383.0;
        GetRealSurface()->SendMidiMessage(GetMidiPressMessage()->midi_message[0], volint&0x7f, (volint>>7)&0x7f);
    }
    
    virtual void SetValueToZero() override
    {
        SetValue(0.0);
    }
    
    void AddToRealSurface(RealSurface* surface) override
    {
        MidiWidget::AddToRealSurface(surface);
        surface->AddWidgetToMessageMap(to_string(GetMidiPressMessage()->midi_message[0]), this);
    }

    virtual void ProcessMidiMessage(const MIDI_event_ex_t* midiMessage) override
    {
        GetRealSurface()->DoAction(GetGUID(), GetActionName(), GetName(), int14ToNormalized(midiMessage->midi_message[2], midiMessage->midi_message[1]));        
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Fader7Bit_MidiWidget : public MidiWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    Fader7Bit_MidiWidget(RealSurface* surface, string name, MIDI_event_ex_t* press, MIDI_event_ex_t* release) : MidiWidget(surface, name, press, release) {}
    
    virtual void SetValue(double value) override
    {
        GetRealSurface()->SendMidiMessage(GetMidiPressMessage()->midi_message[0], GetMidiPressMessage()->midi_message[1], value * 127.0);
    }
    
    virtual void SetValue(double value, int displayMode) override
    {
        SetValue(value);
    }
    
    virtual void SetValueToZero() override
    {
        SetValue(0.0, 0x00);
    }
    
    void AddToRealSurface(RealSurface* surface) override
    {
        MidiWidget::AddToRealSurface(surface);
        surface->AddWidgetToMessageMap(to_string(GetMidiPressMessage()->midi_message[0]) + to_string(GetMidiPressMessage()->midi_message[1]), this);
    }

    virtual void ProcessMidiMessage(const MIDI_event_ex_t* midiMessage) override
    {
        GetRealSurface()->DoAction(GetGUID(), GetActionName(), GetName(), midiMessage->midi_message[2] / 127.0);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Encoder_MidiWidget : public MidiWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    Encoder_MidiWidget(RealSurface* surface, string name, MIDI_event_ex_t* press, MIDI_event_ex_t* release) : MidiWidget(surface, name, press, release) {}
    
    virtual void SetValue(double pan, int displayMode) override
    {
        unsigned char panch = pan * 127.0;
        
        int val = (1+((panch*11)>>7)) | displayMode; // display modes -- 0x00 = line (e.g. pan), 0x01 = boost/cut (e.g. eq), 0x02 = fill from right (e.g. level), 0x03 = center fill (e.g. pan width)
        
        GetRealSurface()->SendMidiMessage(GetMidiPressMessage()->midi_message[0], GetMidiPressMessage()->midi_message[1] + 0x20, val);
    }
    
    virtual void SetValueToZero() override
    {
        SetValue(0.0, 0x00);
    }
    
    void AddToRealSurface(RealSurface* surface) override
    {
        MidiWidget::AddToRealSurface(surface);
        surface->AddWidgetToMessageMap(to_string(GetMidiPressMessage()->midi_message[0]) + to_string(GetMidiPressMessage()->midi_message[1]), this);
    }
 
    virtual void ProcessMidiMessage(const MIDI_event_ex_t* midiMessage) override
    {
        double value = (midiMessage->midi_message[2] & 0x3f) / 63.0;
        
        if (midiMessage->midi_message[2] & 0x40)
            value = -value;
        
        value += GetRealSurface()->GetActionCurrentNormalizedValue(GetGUID(), GetActionName(),  GetName());
        
        GetRealSurface()->DoAction(GetGUID(), GetActionName(), GetName(), value);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class VUMeter_MidiWidget : public MidiWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
private:
    double minDB_ = 0.0;
    double maxDB_ = 0.0;
    
public:
    VUMeter_MidiWidget(RealSurface* surface, string name, double minDB, double maxDB, MIDI_event_ex_t* press, MIDI_event_ex_t* release) : MidiWidget(surface, name, press, release), minDB_(minDB), maxDB_(maxDB){}
    
    double GetMinDB() override { return minDB_; }
    double GetMaxDB() override { return maxDB_; }
    
    virtual void SetValueToZero() override
    {
        SetValue(0.0);
    }
    
    void SetValue(double value) override
    {
        GetRealSurface()->SendMidiMessage(GetMidiPressMessage()->midi_message[0], GetMidiPressMessage()->midi_message[1], value * 127.0);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GainReductionMeter_MidiWidget : public VUMeter_MidiWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
public:
    GainReductionMeter_MidiWidget(RealSurface* surface, string name, double minDB, double maxDB, MIDI_event_ex_t* press, MIDI_event_ex_t* release) : VUMeter_MidiWidget(surface, name, minDB, maxDB, press, release) {}

    virtual void SetValueToZero() override
    {
        SetValue(1.0);
    }
    
    void SetValue(double value) override
    {
        GetRealSurface()->SendMidiMessage(GetMidiPressMessage()->midi_message[0], GetMidiPressMessage()->midi_message[1], fabs(1.0 - value) * 127.0);
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DisplayUpper_MidiWidget : public MidiWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
    int slotIndex_ = 0;
    
public:
    DisplayUpper_MidiWidget(RealSurface* surface, string name, int slotIndex) : MidiWidget(surface, name, new MIDI_event_ex_t(0x00, 0x00, 0x00), new MIDI_event_ex_t(0x00, 0x00, 0x00)), slotIndex_(slotIndex) {}
    
    virtual void SetValueToZero() override
    {
        SetValue("");
    }
    
    void SetValue(string displayText) override
    {
        if(slotIndex_ > 7) // GAW TDB -- this is a hack to prevent Fader 9 (Master) on MCU from displaying on lower row of channel 1
            return;
        
        int pad = 7;
        const char* text = displayText.c_str();
        
        struct
        {
            MIDI_event_ex_t evt;
            char data[512];
        } midiSysExData;
        midiSysExData.evt.frame_offset=0;
        midiSysExData.evt.size=0;
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = 0xF0;
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = 0x00;
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = 0x00;
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = 0x66;
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = 0x14;
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = 0x12;
        
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = slotIndex_ * 7;
        
        int l = strlen(text);
        if (pad < l)
            l = pad;
        if (l > 200)
            l = 200;
        
        int cnt = 0;
        while (cnt < l)
        {
            midiSysExData.evt.midi_message[midiSysExData.evt.size++] = *text++;
            cnt++;
        }
        
        while (cnt++ < pad)
            midiSysExData.evt.midi_message[midiSysExData.evt.size++] = ' ';
        
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = 0xF7;
        
        GetRealSurface()->SendMidiMessage(&midiSysExData.evt);
    }
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DisplayLower_MidiWidget : public MidiWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
    int slotIndex_ = 0;
    
public:
    DisplayLower_MidiWidget(RealSurface* surface, string name, int slotIndex) : MidiWidget(surface, name, new MIDI_event_ex_t(0x00, 0x00, 0x00), new MIDI_event_ex_t(0x00, 0x00, 0x00)), slotIndex_(slotIndex) {}
    
    virtual void SetValueToZero() override
    {
        SetValue("");
    }
    
    void SetValue(string displayText) override
    {
        int pad = 7;
        const char* text = displayText.c_str();
        
        struct
        {
            MIDI_event_ex_t evt;
            char data[512];
        } midiSysExData;
        midiSysExData.evt.frame_offset=0;
        midiSysExData.evt.size=0;
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = 0xF0;
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = 0x00;
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = 0x00;
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = 0x66;
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = 0x14;
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = 0x12;
        
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = slotIndex_ * 7 + 56;
        
        int l = strlen(text);
        if (pad < l)
            l = pad;
        if (l > 200)
            l = 200;
        
        int cnt = 0;
        while (cnt < l)
        {
            midiSysExData.evt.midi_message[midiSysExData.evt.size++] = *text++;
            cnt++;
        }
        
        while (cnt++ < pad)
            midiSysExData.evt.midi_message[midiSysExData.evt.size++] = ' ';
        
        midiSysExData.evt.midi_message[midiSysExData.evt.size++] = 0xF7;
        
        GetRealSurface()->SendMidiMessage(&midiSysExData.evt);
    }
};

#endif /* control_surface_midi_widgets_h */
