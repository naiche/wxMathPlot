// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "../../mathplot.h"

#include <wx/image.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/log.h>
#include <wx/intl.h>

#include <math.h>

double ld(const double x) { return log(x)/log(2.0f); };

// derived classes

class Elias;
class MyFrame;
class MyApp;

class FixedBitwidth : public mpFX //mpAX
{
    int m_bitwidth;
public:
    FixedBitwidth(int bitwidth) : mpFX( wxT("Feste Bitbreite")) { m_bitwidth = bitwidth; }

    virtual double GetY( double /*int ?? */ x )
    {
        if (x>=0)
            return m_bitwidth;
        else
            return 0;
    }
};

class Optimum : public mpFX
{
public:
    Optimum() : mpFX( wxT("Optimum ld(N)")) {}

    virtual double GetY( double x )
    {
        if (x>=1)
            return ld(x);
        else
            return 0;
    }
};

class Elias : public mpFX //mpAX
{
public:
    Elias() : mpFX( wxT("Elias-Kodes")) {}

    virtual double GetY( double /*int ??*/ x )
    {
        if (x>=1)
            return floor(ld(x)) + 2.0 * floor(ld( 1.0 + floor(ld(x))));
        else
            return 0;
    }
};

class Fibonacci : public mpFX //mpAX
{
public:
    Fibonacci() : mpFX( wxT("Fibonacci-Kodes")) {}

    virtual double GetY( double N )
    {
        if (N>=1)
        {
        /*
            for (DWORD mask = 0; mask<N; mask++)
            {
                for (int i=0; i<N; i++)
                {
                    if (mask 

                }
            }
        */

            return N;
        }
        else
            return 0;
    }
};


// MyFrame

class MyFrame: public wxFrame
{
public:
    MyFrame();

    void OnAbout( wxCommandEvent &event );
    void OnQuit( wxCommandEvent &event );
    void OnFit( wxCommandEvent &event );
    void OnZoomIn( wxCommandEvent &event );
    void OnZoomOut( wxCommandEvent &event );
    void OnBlackTheme(wxCommandEvent& event);

    mpWindow        *m_plot;
    wxTextCtrl      *m_log;

private:
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
    ID_BLACK_THEME,
};

IMPLEMENT_DYNAMIC_CLASS( MyFrame, wxFrame )

BEGIN_EVENT_TABLE(MyFrame,wxFrame)
	EVT_MENU(ID_ABOUT, MyFrame::OnAbout)
	EVT_MENU(ID_QUIT,  MyFrame::OnQuit)
	EVT_MENU(mpID_FIT, MyFrame::OnFit)
	EVT_MENU(mpID_ZOOM_IN, MyFrame::OnZoomIn)
	EVT_MENU(mpID_ZOOM_OUT, MyFrame::OnZoomOut)
	EVT_MENU(ID_BLACK_THEME, MyFrame::OnBlackTheme)
END_EVENT_TABLE()

MyFrame::MyFrame() : wxFrame( (wxFrame *)NULL, -1, wxT("wxWindows mathplot sample - Comparison of Integer Coding Algorithms"), wxDefaultPosition, wxSize(1000, 800))
{
    wxMenu *file_menu = new wxMenu();
    wxMenu *view_menu = new wxMenu();

    file_menu->Append( ID_ABOUT, wxT("&About.."));
    file_menu->Append( ID_QUIT,  wxT("E&xit\tAlt-X"));

    view_menu->Append( mpID_FIT,      wxT("&Fit bounding box"), wxT("Set plot view to show all items"));
    view_menu->Append( mpID_ZOOM_IN,  wxT("Zoom in"),           wxT("Zoom in plot view."));
    view_menu->Append( mpID_ZOOM_OUT, wxT("Zoom out"),          wxT("Zoom out plot view."));
    view_menu->AppendCheckItem( ID_BLACK_THEME, wxT("Switch to black background theme"));

    wxMenuBar *menu_bar = new wxMenuBar();
    menu_bar->Append(file_menu, wxT("&File"));
    menu_bar->Append(view_menu, wxT("&View"));

    SetMenuBar( menu_bar );

    mpLayer *e, *f; //, *h, *bs;

    m_plot = new mpWindow( this, -1, wxPoint(0,0), wxSize(100,100), wxSUNKEN_BORDER );
    m_plot->AddLayer(     new mpScaleX( wxT("Ganzzahl N")));
    m_plot->AddLayer(     new mpScaleY( wxT("Kosten K(N) in Bits")));
    
    m_plot->AddLayer(     new FixedBitwidth(32) );
    m_plot->AddLayer(     new Optimum() );
    m_plot->AddLayer( e = new Elias() );
    m_plot->AddLayer( f = new Fibonacci() );
  
    e->SetPen( wxPen(*wxRED, 1, wxPENSTYLE_SOLID));
    f->SetPen( wxPen(*wxGREEN, 1, wxPENSTYLE_SOLID));

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
    topsizer->Add( m_plot, 1, wxEXPAND );
    SetAutoLayout( TRUE );
    SetSizer( topsizer );
    
	m_plot->BindMouseButton(mpDOUBLE_CLICK, mpFIT);
	m_plot->BindMouseButton(mpMIDDLE_DOWN, mpPAN);
	//m_plot->BindMouseButton(mpRIGHT_DOWN, mpCONTEXT_MENU);
	m_plot->BindMouseButton(mpRIGHT_DOWN, mpTRACK);
	m_plot->BindMouseButton(mpLEFT_DOWN, mpZOOM_RECTANGLE);
	
	m_plot->BindMouseWheel(mpWHEEL, mpZOOM);
	m_plot->BindMouseWheel(mpSHIFT_WHEEL, mpHORIZONTAL_PAN);
	m_plot->BindMouseWheel(mpCTRL_WHEEL, mpVERTICAL_PAN);

	wxColour grey(96, 96, 96);
	m_plot->SetColourTheme(*wxWHITE, *wxBLACK, grey, wxColour(220,220,220));
	m_plot->Fit(-1, 15, -1, 16);        
}

void MyFrame::OnQuit( wxCommandEvent &WXUNUSED(event) )
{
	Close( TRUE );
}

void MyFrame::OnFit( wxCommandEvent &WXUNUSED(event) )
{
	m_plot->Fit(-1, 15, -1, 16);
}

void MyFrame::OnZoomIn( wxCommandEvent &WXUNUSED(event) )
{
	m_plot->ZoomIn();
}

void MyFrame::OnZoomOut( wxCommandEvent &WXUNUSED(event) )
{
	m_plot->ZoomOut();
}

void MyFrame::OnBlackTheme(wxCommandEvent& event)
{
	//wxColor black(0,0,0);
	//wxColor white(255,255,255);
	wxColour grey(96, 96, 96);
	if (event.IsChecked()){	
		//wxBrush* brush = new wxBrush(*wxTRANSPARENT_BRUSH);
		//SetColourTheme(background, foreground, axes, grids)	   
		m_plot->SetColourTheme(wxColour(25,25,25), *wxWHITE, grey, wxColour(45,45,45));

		if (m_plot->GetLayerByName("legend") != nullptr)
			m_plot->GetLayerByName("legend")->SetBrush(wxBrush(wxColour(25,25,25), wxBRUSHSTYLE_SOLID));
		if (m_plot->GetLayerByName("coord") != nullptr)
			m_plot->GetLayerByName("coord")->SetBrush(wxBrush(wxColour(65,65,65), wxBRUSHSTYLE_SOLID));
	}
	else
	{
		m_plot->SetColourTheme(*wxWHITE, *wxBLACK, grey, wxColour(220,220,220));
		if (m_plot->GetLayerByName("legend") != nullptr)
			m_plot->GetLayerByName("legend")->SetBrush(wxBrush(*wxWHITE, wxBRUSHSTYLE_SOLID));
		if (m_plot->GetLayerByName("coord") != nullptr)
			m_plot->GetLayerByName("coord")->SetBrush(wxBrush(wxColour(185,185,185), wxBRUSHSTYLE_SOLID));
	}
	m_plot->UpdateAll();
}

void MyFrame::OnAbout( wxCommandEvent &WXUNUSED(event) )
{
    wxMessageBox( wxT("wxWindows mathplot sample\n(?) 2003 David Schalig"));
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

