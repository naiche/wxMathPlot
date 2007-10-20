// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "../../mathplot.h"

#include "wx/image.h"
#include "wx/listctrl.h"
#include "wx/sizer.h"
#include "wx/log.h"
#include "wx/intl.h"
#include "wx/print.h"

#include <math.h>
// #include <time.h>

// derived classes

class MySIN;
class MyCOSinverse;
class MyLissajoux;
class MyFrame;
class MyApp;

// MySIN

class MySIN : public mpFX
{
    double m_freq, m_amp;
public:
    MySIN(double freq, double amp) : mpFX( wxT("f(x) = SIN(x)")) { m_freq=freq; m_amp=amp; }
    virtual double GetY( double x ) { return m_amp * sin(x/6.283185/m_freq); }
    virtual double GetMinY() { return -m_amp; }
    virtual double GetMaxY() { return  m_amp; }
};

// MyCOSinverse

class MyCOSinverse : public mpFY
{
    double m_freq, m_amp;
public:
    MyCOSinverse(double freq, double amp) : mpFY( wxT("g(y) = COS(y)")) { m_freq=freq; m_amp=amp; }
    virtual double GetX( double y ) { return m_amp * cos(y/6.283185/m_freq); }
    virtual double GetMinX() { return -m_amp; }
    virtual double GetMaxX() { return  m_amp; }
};

// MyLissajoux

class MyLissajoux : public mpFXY
{
    double m_rad;
    int    m_idx;
public:
    MyLissajoux(double rad) : mpFXY( wxT("Lissajoux")) { m_rad=rad; m_idx=0; }
    virtual bool GetNextXY( double & x, double & y )
    {
        if (m_idx < 360)
        {
            x = m_rad * cos(m_idx / 6.283185*360);
            y = m_rad * sin(m_idx / 6.283185*360*3);
            m_idx++;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    virtual void Rewind() { m_idx=0; }
    virtual double GetMinX() { return -m_rad; }
    virtual double GetMaxX() { return  m_rad; }
    virtual double GetMinY() { return -m_rad; }
    virtual double GetMaxY() { return  m_rad; }
};



// MyFrame

class MyFrame: public wxFrame
{
public:
    MyFrame();

    void OnAbout( wxCommandEvent &event );
    void OnQuit( wxCommandEvent &event );
    void OnPrintPreview( wxCommandEvent &event);
    void OnPrint( wxCommandEvent &event );
    void OnFit( wxCommandEvent &event );
    void OnAlignXAxis( wxCommandEvent &event );
    void OnAlignYAxis( wxCommandEvent &event );
    void OnToggleGrid( wxCommandEvent &event );

    mpWindow        *m_plot;
    wxTextCtrl      *m_log;

private:
    int axesPos[2];
    bool ticks;
    DECLARE_DYNAMIC_CLASS(MyFrame)
    DECLARE_EVENT_TABLE()
};

// MyApp

class MyApp: public wxApp
{
public:
    virtual bool OnInit();
};

// main program

IMPLEMENT_APP(MyApp)

// MyFrame

enum {
    ID_QUIT  = 108,
    ID_ABOUT,
    ID_PRINT,
    ID_PRINT_PREVIEW,
    ID_ALIGN_X_AXIS,
    ID_ALIGN_Y_AXIS,
    ID_TOGGLE_GRID
};

IMPLEMENT_DYNAMIC_CLASS( MyFrame, wxFrame )

BEGIN_EVENT_TABLE(MyFrame,wxFrame)
  EVT_MENU(ID_ABOUT, MyFrame::OnAbout)
  EVT_MENU(ID_QUIT,  MyFrame::OnQuit)
  EVT_MENU(ID_PRINT_PREVIEW, MyFrame::OnPrintPreview)
  EVT_MENU(ID_PRINT, MyFrame::OnPrint)
  EVT_MENU(mpID_FIT, MyFrame::OnFit)
  EVT_MENU(ID_ALIGN_X_AXIS, MyFrame::OnAlignXAxis)
  EVT_MENU(ID_ALIGN_Y_AXIS, MyFrame::OnAlignYAxis)
  EVT_MENU(ID_TOGGLE_GRID, MyFrame::OnToggleGrid)
END_EVENT_TABLE()

MyFrame::MyFrame()
       : wxFrame( (wxFrame *)NULL, -1, wxT("wxWindows mathplot sample"))
{
    wxMenu *file_menu = new wxMenu();
    wxMenu *view_menu = new wxMenu();

    file_menu->Append( ID_PRINT_PREVIEW, wxT("Print Pre&view..."));
    file_menu->Append( ID_PRINT, wxT("&Print..."));
    file_menu->AppendSeparator();
    file_menu->Append( ID_ABOUT, wxT("&About..."));
    file_menu->Append( ID_QUIT,  wxT("E&xit\tAlt-X"));

    view_menu->Append( mpID_FIT,      wxT("&Fit bounding box"), wxT("Set plot view to show all items"));
    view_menu->Append( mpID_ZOOM_IN,  wxT("Zoom in"),           wxT("Zoom in plot view."));
    view_menu->Append( mpID_ZOOM_OUT, wxT("Zoom out"),          wxT("Zoom out plot view."));
    view_menu->AppendSeparator();
    view_menu->Append( ID_ALIGN_X_AXIS, wxT("Switch &X axis align"));
    view_menu->Append( ID_ALIGN_Y_AXIS, wxT("Switch &Y axis align"));
    view_menu->Append( ID_TOGGLE_GRID, wxT("Toggle grid/ticks"));
    
    wxMenuBar *menu_bar = new wxMenuBar();
    menu_bar->Append(file_menu, wxT("&File"));
    menu_bar->Append(view_menu, wxT("&View"));

    SetMenuBar( menu_bar );
    CreateStatusBar(1);

    mpLayer* l;

    m_plot = new mpWindow( this, -1, wxPoint(0,0), wxSize(100,100), wxSUNKEN_BORDER );
    m_plot->AddLayer(     new mpScaleX(wxT("x"), mpALIGN_BOTTOM, true) );
    m_plot->AddLayer(     new mpScaleY(wxT("y"), mpALIGN_LEFT, true) );
    m_plot->AddLayer(     new MySIN( 50.0, 220.0 ) );
    m_plot->AddLayer(     new MyCOSinverse( 50.0, 100.0 ) );
    m_plot->AddLayer( l = new MyLissajoux( 125.0 ) );
    m_plot->AddLayer(     new mpText(wxT("mpText sample"), 10, 10) );
  
    // set a nice pen for the lissajoux
    wxPen mypen(*wxRED, 5, wxSOLID);
    l->SetPen( mypen);

    m_log = new wxTextCtrl( this, -1, wxT("This is the log window.\n"), wxPoint(0,0), wxSize(100,100), wxTE_MULTILINE );
    wxLog *old_log = wxLog::SetActiveTarget( new wxLogTextCtrl( m_log ) );
    delete old_log;
    
    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    topsizer->Add( m_plot, 1, wxEXPAND );
    topsizer->Add( m_log, 0, wxEXPAND );

    SetAutoLayout( TRUE );
    SetSizer( topsizer );
    axesPos[0] = 0;
    axesPos[1] = 0;
    ticks = true;

	m_plot->EnableDoubleBuffer(true);
}

void MyFrame::OnQuit( wxCommandEvent &WXUNUSED(event) )
{
    Close( TRUE );
}

void MyFrame::OnFit( wxCommandEvent &WXUNUSED(event) )
{
    m_plot->Fit();
}

void MyFrame::OnAbout( wxCommandEvent &WXUNUSED(event) )
{
    wxMessageBox( wxT("wxWidgets mathplot sample\n(c) 2003 David Schalig\n(c) 2007 Davide Rondini"));
}

void MyFrame::OnAlignXAxis( wxCommandEvent &WXUNUSED(event) )
{
    axesPos[0] = (int) (axesPos[0]+1)%5;
    wxString temp;
    temp.sprintf(wxT("axesPos = %d\n"), axesPos);
    m_log->AppendText(temp);
    if (axesPos[0] == 0)
        ((mpScaleX*)(m_plot->GetLayer(0)))->SetAlign(mpALIGN_BORDER_BOTTOM);
    if (axesPos[0] == 1)
        ((mpScaleX*)(m_plot->GetLayer(0)))->SetAlign(mpALIGN_BOTTOM);
    if (axesPos[0] == 2)
        ((mpScaleX*)(m_plot->GetLayer(0)))->SetAlign(mpALIGN_CENTER);
    if (axesPos[0] == 3)
        ((mpScaleX*)(m_plot->GetLayer(0)))->SetAlign(mpALIGN_TOP);
    if (axesPos[0] == 4)
        ((mpScaleX*)(m_plot->GetLayer(0)))->SetAlign(mpALIGN_BORDER_TOP);
    m_plot->UpdateAll();
}

void MyFrame::OnAlignYAxis( wxCommandEvent &WXUNUSED(event) )
{
    axesPos[1] = (int) (axesPos[1]+1)%5;
    wxString temp;
    temp.sprintf(wxT("axesPos = %d\n"), axesPos);
    m_log->AppendText(temp);
    if (axesPos[1] == 0)
        ((mpScaleY*)(m_plot->GetLayer(1)))->SetAlign(mpALIGN_BORDER_LEFT);
    if (axesPos[1] == 1)
        ((mpScaleY*)(m_plot->GetLayer(1)))->SetAlign(mpALIGN_LEFT);
    if (axesPos[1] == 2)
        ((mpScaleY*)(m_plot->GetLayer(1)))->SetAlign(mpALIGN_CENTER);
    if (axesPos[1] == 3)
        ((mpScaleY*)(m_plot->GetLayer(1)))->SetAlign(mpALIGN_RIGHT);
    if (axesPos[1] == 4)
        ((mpScaleY*)(m_plot->GetLayer(1)))->SetAlign(mpALIGN_BORDER_RIGHT);
    m_plot->UpdateAll();
}

void MyFrame::OnToggleGrid( wxCommandEvent &WXUNUSED(event) )
{
    ticks = !ticks;
    ((mpScaleX*)(m_plot->GetLayer(0)))->SetTicks(ticks);
    ((mpScaleY*)(m_plot->GetLayer(1)))->SetTicks(ticks);
    m_plot->UpdateAll();
}

void MyFrame::OnPrintPreview( wxCommandEvent &event)
{
      // Pass two printout objects: for preview, and possible printing.
      mpPrintout *plotPrint = new mpPrintout(m_plot);
      mpPrintout *plotPrintPreview = new mpPrintout(m_plot);
      wxPrintPreview *preview = new wxPrintPreview(plotPrintPreview, plotPrint);
      wxPreviewFrame *frame = new wxPreviewFrame(preview, this, wxT("Print Plot"), wxPoint(100, 100), wxSize(600, 650));
      frame->Centre(wxBOTH);
      frame->Initialize();
      frame->Show(true);
}

void MyFrame::OnPrint( wxCommandEvent &event )
{
      wxPrinter printer;
      mpPrintout printout(m_plot, wxT("Plot print"));
      printer.Print(this, &printout, true);
}


//-----------------------------------------------------------------------------
// MyApp
//-----------------------------------------------------------------------------

bool MyApp::OnInit()
{
    wxFrame *frame = new MyFrame();
    frame->Show( TRUE );

    return TRUE;
}

