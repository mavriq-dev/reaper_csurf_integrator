//
//  control_surface_integrator_ui.cpp
//  reaper_csurf_integrator
//
//

#include "control_surface_integrator.h"
#include "control_surface_integrator_ui.h"

extern REAPER_PLUGIN_HINSTANCE g_hInst;

////////////////////////////////////////////////////////////////////////////////////////////////////////
// structs
////////////////////////////////////////////////////////////////////////////////////////////////////////
struct RealSurfaceLine
{
    string name = "";
    int numChannels = 0;
    bool isBankable = true;
    int midiIn = 0;
    int midiOut = 0;
    string templateFilename = "";
};

struct VirtualSurfaceLine
{
    string realSurfaceName = "";
    string actionTemplateFolder = "";
    string FXTemplateFolder = "";
};

struct ZoneLine
{
    string name = "";
    bool followMCP = true;
    vector<VirtualSurfaceLine*> virtualSurfaces;
};

struct LayerLine
{
    string name = "";
    vector<ZoneLine*> zones;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
// CSurfIntegrator
////////////////////////////////////////////////////////////////////////////////////////////////////////
static CSurfIntegrator* integrator = nullptr;

CSurfIntegrator::CSurfIntegrator()
{
    manager_ = new CSurfManager();
}

CSurfIntegrator::~CSurfIntegrator()
{
    delete manager_;
}

void CSurfIntegrator::OnTrackSelection(MediaTrack *trackid)
{
    manager_->OnTrackSelection(trackid);
}

int CSurfIntegrator::Extended(int call, void *parm1, void *parm2, void *parm3)
{
    if(call == CSURF_EXT_SUPPORTS_EXTENDED_TOUCH)
    {
        return 1;
    }
    
    if(call == CSURF_EXT_RESET)
    {
        if(integrator)
            integrator->GetManager()->ReInit();
    }
    
    if(call == CSURF_EXT_SETFXCHANGE)
    {
        // parm1=(MediaTrack*)track, whenever FX are added, deleted, or change order
        manager_->TrackFXListChanged((MediaTrack*)parm1);
    }

    return 1;
}

bool CSurfIntegrator::GetTouchState(MediaTrack *track, int touchedControl)
{
    return manager_->GetTouchState(track, touchedControl);
}

void CSurfIntegrator::Run()
{
    manager_->Run();
}

void CSurfIntegrator::SetTrackListChange()
{
    manager_->TrackListChanged();
}

const char *CSurfIntegrator::GetTypeString()
{
    return "CSI";
}

const char *CSurfIntegrator::GetDescString()
{
    descspace.Set(Control_Surface_Integrator.c_str());
    return descspace.Get();
}

const char *CSurfIntegrator::GetConfigString() // string of configuration data
{
    sprintf(configtmp,"0 0");
    return configtmp;
}

static IReaperControlSurface *createFunc(const char *type_string, const char *configString, int *errStats)
{
    integrator = new CSurfIntegrator();
    return integrator;
}

void AddComboEntry(HWND hwndDlg, int x, char * buf, int comboId)
{
    int a=SendDlgItemMessage(hwndDlg,comboId,CB_ADDSTRING,0,(LPARAM)buf);
    SendDlgItemMessage(hwndDlg,comboId,CB_SETITEMDATA,a,x);
}

void AddListEntry(HWND hwndDlg, string buf, int comboId)
{
    SendDlgItemMessage(hwndDlg, comboId, LB_ADDSTRING, 0, (LPARAM)buf.c_str());
}

vector<RealSurfaceLine*> realSurfaces;
vector<LayerLine*> layers;

bool editMode = false;
static int dlgResult = 0;

static char name[BUFSZ];
static int numChannels = 0;
static bool isBankable = true;
static bool followMCP = true;
static int midiIn = 0;
static int midiOut = 0;
static char templateFilename[BUFSZ];
static char actionTemplateFolder[BUFSZ];
static char FXTemplateFolder[BUFSZ];

static WDL_DLGRET dlgProcLayer(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            if(editMode)
            {
                editMode = false;
                SetDlgItemText(hwndDlg, IDC_EDIT_LayerName, name);
            }
        }
            
        case WM_COMMAND:
            {
                switch(LOWORD(wParam))
                {
                    case IDOK:
                        if (HIWORD(wParam) == BN_CLICKED)
                        {
                            GetDlgItemText(hwndDlg, IDC_EDIT_LayerName, name, sizeof(name));
                            dlgResult = IDOK;
                            EndDialog(hwndDlg, 0);
                        }
                        break ;
                        
                    case IDCANCEL:
                        if (HIWORD(wParam) == BN_CLICKED)
                            EndDialog(hwndDlg, 0);
                        break ;
                }
            }
            break ;
            
        case WM_CLOSE:
            DestroyWindow(hwndDlg);
            break ;
            
        case WM_DESTROY:
            EndDialog(hwndDlg, 0);
            break;
            
        default:
            return DefWindowProc(hwndDlg, uMsg, wParam, lParam) ;
    }
    
    return 0 ;
}

static WDL_DLGRET dlgProcRealSurface(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            string path(DAW::GetResourcePath());
            path += "/CSI/rst/";
            int i = 0;
            for(auto filename : FileSystem::GetDirectoryFilenames(path))
            {
                int length = filename.length();
                if(length > 4 && filename[0] != '.' && filename[length - 4] == '.' && filename[length - 3] == 'r' && filename[length - 2] == 's' &&filename[length - 1] == 't')
                    AddComboEntry(hwndDlg, i++, (char*)filename.c_str(), IDC_COMBO_SurfaceTemplate);
            }

            char buf[BUFSZ];
            int currentIndex = 0;

            for (int i = 0; i < GetNumMIDIInputs(); i++)
                if (GetMIDIInputName(i, buf, sizeof(buf)))
                {
                    AddComboEntry(hwndDlg, i, buf, IDC_COMBO_MidiIn);
                    if(editMode && midiIn == i)
                        SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_MidiIn), CB_SETCURSEL, currentIndex, 0);
                    currentIndex++;
                }
            
            currentIndex = 0;
            
            for (int i = 0; i < GetNumMIDIOutputs(); i++)
                if (GetMIDIOutputName(i, buf, sizeof(buf)))
                {
                    AddComboEntry(hwndDlg, i, buf, IDC_COMBO_MidiOut);
                    if(editMode && midiOut == i)
                        SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_MidiOut), CB_SETCURSEL, currentIndex, 0);
                    currentIndex++;
                }
            
            if(editMode)
            {
                editMode = false;
                SetDlgItemText(hwndDlg, IDC_EDIT_RealSurfaceName, name);
                SetDlgItemText(hwndDlg, IDC_EDIT_RealSurfaceNumChannels, to_string(numChannels).c_str());
                
                if(isBankable)
                    CheckDlgButton(hwndDlg, IDC_CHECK_IsBankable, BST_CHECKED);
                else
                    CheckDlgButton(hwndDlg, IDC_CHECK_IsBankable, BST_UNCHECKED);

                
                
                int index = SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_SurfaceTemplate), CB_FINDSTRING, -1, (LPARAM)templateFilename);
                if(index >= 0)
                    SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_SurfaceTemplate), CB_SETCURSEL, index, 0);
            }
            else
            {
                CheckDlgButton(hwndDlg, IDC_CHECK_IsBankable, BST_CHECKED);
                SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_SurfaceTemplate), CB_SETCURSEL, 0, 0);
                SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_MidiIn), CB_SETCURSEL, 0, 0);
                SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_MidiOut), CB_SETCURSEL, 0, 0);
            }
        }

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDOK:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        GetDlgItemText(hwndDlg, IDC_EDIT_RealSurfaceName, name, sizeof(name));
                        char tempBuf[BUFSZ];
                        GetDlgItemText(hwndDlg, IDC_EDIT_RealSurfaceNumChannels, tempBuf, sizeof(tempBuf));
                        numChannels = atoi(tempBuf);
                        if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_IsBankable))
                            isBankable = true;
                        GetDlgItemText(hwndDlg, IDC_COMBO_SurfaceTemplate, templateFilename, sizeof(templateFilename));
                        
                        int currentSelection = SendDlgItemMessage(hwndDlg, IDC_COMBO_MidiIn, CB_GETCURSEL, 0, 0);
                        if (currentSelection >= 0)
                            midiIn = SendDlgItemMessage(hwndDlg, IDC_COMBO_MidiIn, CB_GETITEMDATA, currentSelection, 0);
                        currentSelection = SendDlgItemMessage(hwndDlg, IDC_COMBO_MidiOut, CB_GETCURSEL, 0, 0);
                        if (currentSelection >= 0)
                            midiOut = SendDlgItemMessage(hwndDlg, IDC_COMBO_MidiOut, CB_GETITEMDATA, currentSelection, 0);

                        dlgResult = IDOK;
                        EndDialog(hwndDlg, 0);
                    }
                    break ;
                    
                case IDCANCEL:
                    if (HIWORD(wParam) == BN_CLICKED)
                        EndDialog(hwndDlg, 0);
                    break ;
            }
        }
            break ;
            
        case WM_CLOSE:
            DestroyWindow(hwndDlg) ;
            break ;
            
        case WM_DESTROY:
            EndDialog(hwndDlg, 0);
            break;
            
        default:
            return DefWindowProc(hwndDlg, uMsg, wParam, lParam) ;
    }
    
    return 0 ;
}

static WDL_DLGRET dlgProcZone(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            if(followMCP)
                CheckDlgButton(hwndDlg, IDC_RADIO_MCP, BST_CHECKED);
            else
                CheckDlgButton(hwndDlg, IDC_RADIO_TCP, BST_CHECKED);

            if(editMode)
            {
                editMode = false;
                SetDlgItemText(hwndDlg, IDC_EDIT_ZoneName, name);
            }
        }

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDC_RADIO_MCP:
                    CheckDlgButton(hwndDlg, IDC_RADIO_TCP, BST_UNCHECKED);
                    break;
                    
                case IDC_RADIO_TCP:
                    CheckDlgButton(hwndDlg, IDC_RADIO_MCP, BST_UNCHECKED);
                    break;

                case IDOK:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        GetDlgItemText(hwndDlg, IDC_EDIT_ZoneName , name, sizeof(name));
                        if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_MCP))
                            followMCP = true;
                        else
                            followMCP = false;
                        dlgResult = IDOK;
                        EndDialog(hwndDlg, 0);
                    }
                    break ;
                    
                case IDCANCEL:
                    if (HIWORD(wParam) == BN_CLICKED)
                        EndDialog(hwndDlg, 0);
                    break ;
            }
        }
            break ;
            
        case WM_CLOSE:
            DestroyWindow(hwndDlg) ;
            break ;
            
        case WM_DESTROY:
            EndDialog(hwndDlg, 0);
            break;
            
        default:
            return DefWindowProc(hwndDlg, uMsg, wParam, lParam) ;
    }
    
    return 0 ;
}

static WDL_DLGRET dlgProcVirtualSurface(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            for(auto* surface :  realSurfaces)
                AddComboEntry(hwndDlg, 0, (char *)surface->name.c_str(), IDC_COMBO_RealSurface);
            
            string resourcePath(DAW::GetResourcePath());
            resourcePath += "/CSI/";
            
            for(auto foldername : FileSystem::GetDirectoryFolderNames(resourcePath + "axt/"))
                if(foldername[0] != '.')
                    AddComboEntry(hwndDlg, 0, (char *)foldername.c_str(), IDC_COMBO_ActionTemplates);

            for(auto foldername : FileSystem::GetDirectoryFolderNames(resourcePath + "fxt/"))
                if(foldername[0] != '.')
                    AddComboEntry(hwndDlg, 0, (char *)foldername.c_str(), IDC_COMBO_FXTemplates);
            
            if(editMode)
            {
                editMode = false;
                int index = SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_RealSurface), CB_FINDSTRING, -1, (LPARAM)name);
                if(index >= 0)
                    SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_RealSurface), CB_SETCURSEL, index, 0);
                
                index = SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_ActionTemplates), CB_FINDSTRING, -1, (LPARAM)actionTemplateFolder);
                if(index >= 0)
                    SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_ActionTemplates), CB_SETCURSEL, index, 0);
                
                index = SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_FXTemplates), CB_FINDSTRING, -1, (LPARAM)FXTemplateFolder);
                if(index >= 0)
                    SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_FXTemplates), CB_SETCURSEL, index, 0);
            }
            else
            {
                SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_RealSurface), CB_SETCURSEL, 0, 0);
                SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_ActionTemplates), CB_SETCURSEL, 0, 0);
                SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_FXTemplates), CB_SETCURSEL, 0, 0);
            }
        }
            
        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDOK:
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        GetDlgItemText(hwndDlg, IDC_COMBO_RealSurface , name, sizeof(name));
                        GetDlgItemText(hwndDlg, IDC_COMBO_ActionTemplates , actionTemplateFolder, sizeof(actionTemplateFolder));
                        GetDlgItemText(hwndDlg, IDC_COMBO_FXTemplates , FXTemplateFolder, sizeof(FXTemplateFolder));
                        dlgResult = IDOK;
                        EndDialog(hwndDlg, 0);
                    }
                    break ;
                    
                case IDCANCEL:
                    if (HIWORD(wParam) == BN_CLICKED)
                        EndDialog(hwndDlg, 0);
                    break ;
            }
        }
            break ;
            
        case WM_CLOSE:
            DestroyWindow(hwndDlg) ;
            break ;
            
        case WM_DESTROY:
            EndDialog(hwndDlg, 0);
            break;
            
        default:
            return DefWindowProc(hwndDlg, uMsg, wParam, lParam) ;
    }
    
    return 0 ;
}

static WDL_DLGRET dlgProcMainConfig(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
            {
                switch(LOWORD(wParam))
                {
                    case IDC_LIST_Layers:
                        if (HIWORD(wParam) == LBN_SELCHANGE)
                        {
                            int index = SendDlgItemMessage(hwndDlg, IDC_LIST_Layers, LB_GETCURSEL, 0, 0);
                            if (index >= 0)
                            {
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Zones), LB_RESETCONTENT, 0, 0);
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_RESETCONTENT, 0, 0);

                                for(auto* zone : layers[index]->zones)
                                    AddListEntry(hwndDlg, zone->name, IDC_LIST_Zones);
                            }
                            else
                            {
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Zones), LB_RESETCONTENT, 0, 0);
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_RESETCONTENT, 0, 0);
                            }
                        }
                        break;
                        
                    case IDC_LIST_Zones:
                        if (HIWORD(wParam) == LBN_SELCHANGE)
                        {
                            int layerIndex = SendDlgItemMessage(hwndDlg, IDC_LIST_Layers, LB_GETCURSEL, 0, 0);
                            int index = SendDlgItemMessage(hwndDlg, IDC_LIST_Zones, LB_GETCURSEL, 0, 0);
                            if (layerIndex >= 0 && index >= 0)
                            {
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_RESETCONTENT, 0, 0);

                                for(auto* layer: layers[layerIndex]->zones[index]->virtualSurfaces)
                                    AddListEntry(hwndDlg, layer->realSurfaceName, IDC_LIST_VirtualSurfaces);
                            }
                            else
                            {
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_RESETCONTENT, 0, 0);
                            }
                        }
                        break;
                        
                    case IDC_BUTTON_AddRealSurface:
                        if (HIWORD(wParam) == BN_CLICKED)
                        {
                            dlgResult = false;
                            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_RealSurface), hwndDlg, dlgProcRealSurface);
                            if(dlgResult == IDOK)
                            {
                                RealSurfaceLine* surface = new RealSurfaceLine();
                                surface->name = name;
                                surface->numChannels = numChannels;
                                surface->isBankable = isBankable;
                                surface->midiIn = midiIn;
                                surface->midiOut = midiOut;
                                surface->templateFilename = templateFilename;
                                realSurfaces.push_back(surface);
                                AddListEntry(hwndDlg, name, IDC_LIST_RealSurfaces);
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_RealSurfaces), LB_SETCURSEL, realSurfaces.size() - 1, 0);
                            }
                        }
                        break ;
                        
                    case IDC_BUTTON_AddVirtualSurface:
                        if (HIWORD(wParam) == BN_CLICKED)
                        {
                            dlgResult = false;
                            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_VirtualSurface), hwndDlg, dlgProcVirtualSurface);
                            if(dlgResult == IDOK)
                            {
                                int layerIndex = SendDlgItemMessage(hwndDlg, IDC_LIST_Layers, LB_GETCURSEL, 0, 0);
                                int zoneIndex = SendDlgItemMessage(hwndDlg, IDC_LIST_Zones, LB_GETCURSEL, 0, 0);
                                
                                if(layerIndex >= 0 && zoneIndex >= 0)
                                {
                                    VirtualSurfaceLine* surface = new VirtualSurfaceLine();
                                    surface->realSurfaceName = name;
                                    surface->actionTemplateFolder = actionTemplateFolder;
                                    surface->FXTemplateFolder = FXTemplateFolder;
                                    layers[layerIndex]->zones[zoneIndex]->virtualSurfaces.push_back(surface);
                                    AddListEntry(hwndDlg, name, IDC_LIST_VirtualSurfaces);
                                    SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_SETCURSEL, layers[layerIndex]->zones[zoneIndex]->virtualSurfaces.size() - 1, 0);
                                }
                            }
                        }
                        break ;
                        
                    case IDC_BUTTON_AddZone:
                        if (HIWORD(wParam) == BN_CLICKED)
                        {
                            int layerIndex = SendDlgItemMessage(hwndDlg, IDC_LIST_Layers, LB_GETCURSEL, 0, 0);
                            if(layerIndex >= 0)
                            {
                                dlgResult = false;
                                followMCP = true;
                                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_Zone), hwndDlg, dlgProcZone);
                                if(dlgResult == IDOK)
                                {
                                    ZoneLine* zone = new ZoneLine();
                                    zone->name = name;
                                    zone->followMCP = followMCP;
                                    layers[layerIndex]->zones.push_back(zone);
                                    AddListEntry(hwndDlg, name, IDC_LIST_Zones);
                                    SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Zones), LB_SETCURSEL, layers[layerIndex]->zones.size() - 1, 0);
                                    SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_RESETCONTENT, 0, 0);
                                }
                            }
                        }
                        break ;
                        
                    case IDC_BUTTON_AddLayer:
                        if (HIWORD(wParam) == BN_CLICKED)
                        {
                            dlgResult = false;
                            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_Layer), hwndDlg, dlgProcLayer);
                            if(dlgResult == IDOK)
                            {
                                LayerLine* layer = new LayerLine();
                                layer->name = name;
                                layers.push_back(layer);
                                AddListEntry(hwndDlg, name, IDC_LIST_Layers);
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Layers), LB_SETCURSEL, layers.size() - 1, 0);
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Zones), LB_RESETCONTENT, 0, 0);
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_RESETCONTENT, 0, 0);
                            }
                        }
                        break ;
                        
                    case IDC_BUTTON_EditRealSurface:
                        if (HIWORD(wParam) == BN_CLICKED)
                        {
                            int index = SendDlgItemMessage(hwndDlg, IDC_LIST_RealSurfaces, LB_GETCURSEL, 0, 0);
                            if(index >= 0)
                            {
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_RealSurfaces), LB_GETTEXT, index, (LPARAM)(LPCTSTR)(name));
                                numChannels = realSurfaces[index]->numChannels;
                                isBankable = realSurfaces[index]->isBankable;
                                midiIn = realSurfaces[index]->midiIn;
                                midiOut = realSurfaces[index]->midiOut;
                                strcpy(templateFilename, realSurfaces[index]->templateFilename.c_str());
                                dlgResult = false;
                                editMode = true;
                                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_RealSurface), hwndDlg, dlgProcRealSurface);
                                if(dlgResult == IDOK)
                                {
                                    realSurfaces[index]->name = name;
                                    realSurfaces[index]->numChannels = numChannels;
                                    realSurfaces[index]->isBankable = isBankable;
                                    realSurfaces[index]->midiIn = midiIn;
                                    realSurfaces[index]->midiOut = midiOut;
                                    realSurfaces[index]->templateFilename = templateFilename;
                                    
                                    SendMessage(GetDlgItem(hwndDlg, IDC_LIST_RealSurfaces), LB_RESETCONTENT, 0, 0);
                                    for(auto* surface: realSurfaces)
                                        AddListEntry(hwndDlg, surface->name, IDC_LIST_RealSurfaces);
                                    SendMessage(GetDlgItem(hwndDlg, IDC_LIST_RealSurfaces), LB_SETCURSEL, index, 0);
                                }
                            }
                        }
                        break ;
                        
                    case IDC_BUTTON_EditVirtualSurface:
                        if (HIWORD(wParam) == BN_CLICKED)
                        {
                            int layerIndex = SendDlgItemMessage(hwndDlg, IDC_LIST_Layers, LB_GETCURSEL, 0, 0);
                            int zoneIndex = SendDlgItemMessage(hwndDlg, IDC_LIST_Zones, LB_GETCURSEL, 0, 0);
                            int index = SendDlgItemMessage(hwndDlg, IDC_LIST_VirtualSurfaces, LB_GETCURSEL, 0, 0);
                            
                            VirtualSurfaceLine* surfaceLine = nullptr;
                            
                            if(layerIndex >= 0 && zoneIndex >= 0 && index >= 0)
                            {
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_GETTEXT, index, (LPARAM)(LPCTSTR)(name));
                                
                                for(auto* surface : layers[layerIndex]->zones[zoneIndex]->virtualSurfaces)
                                    if(surface->realSurfaceName == name)
                                    {
                                        surfaceLine = surface;
                                        
                                        strcpy(actionTemplateFolder, surface->actionTemplateFolder.c_str());
                                        strcpy(FXTemplateFolder, surface->FXTemplateFolder.c_str());
                                    }
                            
                                dlgResult = false;
                                editMode = true;
                                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_VirtualSurface), hwndDlg, dlgProcVirtualSurface);
                                if(dlgResult == IDOK)
                                {
                                    surfaceLine->realSurfaceName = name;
                                    surfaceLine->actionTemplateFolder = actionTemplateFolder;
                                    surfaceLine->FXTemplateFolder = FXTemplateFolder;
                                    
                                    SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_RESETCONTENT, 0, 0);
                                    for(auto* surface: layers[layerIndex]->zones[zoneIndex]->virtualSurfaces)
                                        AddListEntry(hwndDlg, surface->realSurfaceName, IDC_LIST_VirtualSurfaces);
                                    SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_SETCURSEL, index, 0);
                                }
                            }
                        }
                        break ;

                    case IDC_BUTTON_EditZone:
                        if (HIWORD(wParam) == BN_CLICKED)
                        {
                            int layerIndex = SendDlgItemMessage(hwndDlg, IDC_LIST_Layers, LB_GETCURSEL, 0, 0);
                            int index = SendDlgItemMessage(hwndDlg, IDC_LIST_Zones, LB_GETCURSEL, 0, 0);
                            if(layerIndex >= 0 && index >= 0)
                            {
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Zones), LB_GETTEXT, index, (LPARAM)(LPCTSTR)(name));
                                followMCP = layers[layerIndex]->zones[index]->followMCP;
                                dlgResult = false;
                                editMode = true;
                                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_Zone), hwndDlg, dlgProcZone);
                                if(dlgResult == IDOK)
                                {
                                    layers[layerIndex]->zones[index]->name = name;
                                    layers[layerIndex]->zones[index]->followMCP = followMCP;
                                    SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Zones), LB_RESETCONTENT, 0, 0);
                                    for(auto* zone: layers[layerIndex]->zones)
                                        AddListEntry(hwndDlg, zone->name, IDC_LIST_Zones);
                                    SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Zones), LB_SETCURSEL, index, 0);
                                }
                            }
                        }
                        break ;

                    case IDC_BUTTON_EditLayer:
                        if (HIWORD(wParam) == BN_CLICKED)
                        {
                            int index = SendDlgItemMessage(hwndDlg, IDC_LIST_Layers, LB_GETCURSEL, 0, 0);
                            if(index >= 0)
                            {
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Layers), LB_GETTEXT, index, (LPARAM)(LPCTSTR)(name));
                                dlgResult = false;
                                editMode = true;
                                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_Layer), hwndDlg, dlgProcLayer);
                                if(dlgResult == IDOK)
                                {
                                    layers[index]->name = name;
                                    SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Layers), LB_RESETCONTENT, 0, 0);
                                    for(auto* layer: layers)
                                        AddListEntry(hwndDlg, layer->name, IDC_LIST_Layers);
                                    SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Layers), LB_SETCURSEL, index, 0);
                                }
                            }
                        }
                        break ;

                    case IDC_BUTTON_RemoveRealSurface:
                        if (HIWORD(wParam) == BN_CLICKED)
                        {
                            int index = SendDlgItemMessage(hwndDlg, IDC_LIST_RealSurfaces, LB_GETCURSEL, 0, 0);
                            
                            if(index >= 0)
                            {
                                realSurfaces.erase(realSurfaces.begin() + index);
                                
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_RealSurfaces), LB_RESETCONTENT, 0, 0);
                                for(auto* surface: realSurfaces)
                                    AddListEntry(hwndDlg, surface->name, IDC_LIST_RealSurfaces);
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_RealSurfaces), LB_SETCURSEL, index, 0);
                            }
                        }
                        break ;
                        
                    case IDC_BUTTON_RemoveVirtualSurface:
                        if (HIWORD(wParam) == BN_CLICKED)
                        {
                            int layerIndex = SendDlgItemMessage(hwndDlg, IDC_LIST_Layers, LB_GETCURSEL, 0, 0);
                            int zoneIndex = SendDlgItemMessage(hwndDlg, IDC_LIST_Zones, LB_GETCURSEL, 0, 0);
                            int index = SendDlgItemMessage(hwndDlg, IDC_LIST_VirtualSurfaces, LB_GETCURSEL, 0, 0);
                            
                            if(layerIndex >= 0 && zoneIndex >= 0 && index >= 0)
                            {
                                layers[layerIndex]->zones[zoneIndex]->virtualSurfaces.erase(layers[layerIndex]->zones[zoneIndex]->virtualSurfaces.begin() + index);
                                
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_RESETCONTENT, 0, 0);
                                for(auto* surface: layers[layerIndex]->zones[zoneIndex]->virtualSurfaces)
                                    AddListEntry(hwndDlg, surface->realSurfaceName, IDC_LIST_VirtualSurfaces);
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_SETCURSEL, index, 0);
                            }
                        }
                        break ;
                        
                    case IDC_BUTTON_RemoveZone:
                        if (HIWORD(wParam) == BN_CLICKED)
                        {
                            int layerIndex = SendDlgItemMessage(hwndDlg, IDC_LIST_Layers, LB_GETCURSEL, 0, 0);
                            int index = SendDlgItemMessage(hwndDlg, IDC_LIST_Zones, LB_GETCURSEL, 0, 0);
                            if(layerIndex >= 0 && index >= 0)
                            {
                                layers[layerIndex]->zones.erase(layers[layerIndex]->zones.begin() + index);
                                
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Zones), LB_RESETCONTENT, 0, 0);
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_RESETCONTENT, 0, 0);
                                
                                for(auto* zone: layers[layerIndex]->zones)
                                    AddListEntry(hwndDlg, zone->name, IDC_LIST_Zones);
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Zones), LB_SETCURSEL, index, 0);
                            }
                        }
                        break ;

                    case IDC_BUTTON_RemoveLayer:
                        if (HIWORD(wParam) == BN_CLICKED)
                        {
                            int index = SendDlgItemMessage(hwndDlg, IDC_LIST_Layers, LB_GETCURSEL, 0, 0);
                            if(index >= 0)
                            {
                                layers.erase(layers.begin() + index);
                                
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Layers), LB_RESETCONTENT, 0, 0);
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Zones), LB_RESETCONTENT, 0, 0);
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_RESETCONTENT, 0, 0);
                                
                                for(auto* layer: layers)
                                    AddListEntry(hwndDlg, layer->name, IDC_LIST_Layers);
                                
                                SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Layers), LB_SETCURSEL, 0, 0);
                            }
                        }
                        break ;
                }
            }
            break ;
            
        case WM_INITDIALOG:
        {
            realSurfaces.clear();
            layers.clear();
            
            ifstream iniFile(string(DAW::GetResourcePath()) + "/CSI/CSI.ini");
            
            for (string line; getline(iniFile, line) ; )
            {
                if(line[0] != '/' && line != "") // ignore comment lines and blank lines
                {
                    istringstream iss(line);
                    vector<string> tokens;
                    string token;
                    
                    while (iss >> quoted(token))
                        tokens.push_back(token);
                    
                    if(tokens[0] == MidiInMonitor)
                    {
                        if(tokens.size() != 2)
                            continue;
                        
                        if(tokens[1] == "On")
                            CheckDlgButton(hwndDlg, IDC_CHECK_MidiInMon, BST_CHECKED);
                    }
                    else if(tokens[0] == MidiOutMonitor)
                    {
                        if(tokens.size() != 2)
                            continue;
                        
                        if(tokens[1] == "On")
                            CheckDlgButton(hwndDlg, IDC_CHECK_MidiOutMon, BST_CHECKED);
                    }
                    else if(tokens[0] == VSTMonitor)
                    {
                        if(tokens.size() != 2)
                            continue;
                        
                        if(tokens[1] == "On")
                            CheckDlgButton(hwndDlg, IDC_CHECK_VSTParamMon, BST_CHECKED);
                    }
                    else if(tokens[0] == RealSurface_)
                    {
                        if(tokens.size() != 7)
                            continue;
                    
                        RealSurfaceLine* surface = new RealSurfaceLine();
                        surface->name = tokens[1];
                        surface->numChannels = atoi(tokens[2].c_str());
                        surface->isBankable = tokens[3] == "1" ? true : false;
                        surface->midiIn = atoi(tokens[4].c_str());
                        surface->midiOut = atoi(tokens[5].c_str());
                        surface->templateFilename = tokens[6];
                        realSurfaces.push_back(surface);
                        
                        AddListEntry(hwndDlg, surface->name, IDC_LIST_RealSurfaces);
                        
                    }
                    else if(tokens[0] == Layer_)
                    {
                        if(tokens.size() != 2)
                            continue;
                        
                        LayerLine* layer = new LayerLine();
                        layer->name = tokens[1];
                        layers.push_back(layer);
                        
                        AddListEntry(hwndDlg, layer->name, IDC_LIST_Layers);

                    }
                    else if(tokens[0] == Zone_)
                    {
                        if(tokens.size() != 3)
                            continue;
 
                        ZoneLine* zone = new ZoneLine();
                        zone->name = tokens[1];
                        if(tokens[2] == "Yes")
                            zone->followMCP = true;
                        else
                            zone->followMCP = false;
                        layers.back()->zones.push_back(zone);
                    }
                    else if(tokens[0] == VirtualSurface_)
                    {
                        if(tokens.size() != 4)
                            continue;
                        
                        VirtualSurfaceLine* virtualSurface = new VirtualSurfaceLine();
                        virtualSurface->realSurfaceName = tokens[1];
                        virtualSurface->actionTemplateFolder = tokens[2];
                        virtualSurface->FXTemplateFolder = tokens[3];
                        layers.back()->zones.back()->virtualSurfaces.push_back(virtualSurface);
                    }
                }
            }
            
            SendMessage(GetDlgItem(hwndDlg, IDC_LIST_RealSurfaces), LB_SETCURSEL, 0, 0);
            SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Layers), LB_SETCURSEL, 0, 0);
            SendMessage(GetDlgItem(hwndDlg, IDC_LIST_Zones), LB_SETCURSEL, 0, 0);
            SendMessage(GetDlgItem(hwndDlg, IDC_LIST_VirtualSurfaces), LB_SETCURSEL, 0, 0);

        }
        break;
        
        case WM_USER+1024:
        {
            ofstream iniFile(string(DAW::GetResourcePath()) + "/CSI/CSI.ini");

            if(iniFile.is_open())
            {
                string line = MidiInMonitor + " ";
                if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_MidiInMon))
                    line += "On";
                else
                    line += "Off";
                iniFile << line + "\n";
                
                line = MidiOutMonitor + " ";
                if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_MidiOutMon))
                    line += "On";
                else
                    line += "Off";
                iniFile << line + "\n";
                
                line = VSTMonitor + " ";
                if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_VSTParamMon))
                    line += "On";
                else
                    line += "Off";
                iniFile << line + "\n";
                
                iniFile << "\n";
                        
                for(auto surface : realSurfaces)
                {
                    line = RealSurface_ + " ";
                    line += surface->name + " ";
                    line += to_string(surface->numChannels) + " ";
                    line += surface->isBankable ? "1 " : "0 ";
                    line += to_string(surface->midiIn) + " " ;
                    line += to_string(surface->midiOut) + " " ;
                    line += surface->templateFilename + "\n";
                 
                    iniFile << line;
                }

                for(auto layer : layers)
                {
                    iniFile << "\n";
                    
                    line = Layer_ + " ";
                    line += layer->name + "\n";
                    iniFile << line;
                    
                    for(auto zone : layer->zones)
                    {
                        line = Zone_ + " ";
                        line += zone->name + " ";
                        if(zone->followMCP)
                            line += "Yes\n";
                        else
                            line += "No\n";
                        iniFile << line;

                        for(auto virtualSurface : zone->virtualSurfaces)
                        {
                            line = VirtualSurface_ + " ";
                            line += virtualSurface->realSurfaceName + " ";
                            line += virtualSurface->actionTemplateFolder + " " ;
                            line += virtualSurface->FXTemplateFolder + "\n";
                            
                            iniFile << line;
                        }
                    }
                }
                
                iniFile.close();
            }
            
            if(integrator)
                integrator->GetManager()->ReInit();
        }
        break;
    }
    
    return 0;
}

static HWND configFunc(const char *type_string, HWND parent, const char *initConfigString)
{
    return CreateDialogParam(g_hInst,MAKEINTRESOURCE(IDD_SURFACEEDIT_CSI),parent,dlgProcMainConfig,(LPARAM)initConfigString);
}

reaper_csurf_reg_t csurf_integrator_reg =
{
    "CSI",
    Control_Surface_Integrator.c_str(),
    createFunc,
    configFunc,
};
