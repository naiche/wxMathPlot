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
#include <wx/print.h>
#include <wx/filename.h>

#include <math.h>
// #include <time.h>

// wxWidgets 2.6 backward compatibility
#if wxMAJOR_VERSION < 3 && wxMINOR_VERSION < 7
#define wxFD_SAVE wxSAVE
#endif

// Memory leak debugging
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// derived classes

class MyFrame;
class MyApp;

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
	void OnZoomIn( wxCommandEvent &event );
	void OnZoomOut( wxCommandEvent &event );
	void OnToggleTicks( wxCommandEvent &event );
	void OnToggleGrid( wxCommandEvent &event );
	void OnToggleInfoLayer(wxCommandEvent& event);
	void OnSaveScreenshot(wxCommandEvent& event);
	void OnToggleSeries1(wxCommandEvent& event);
	void OnToggleSeries2(wxCommandEvent& event);
	void OnToggleSeries3(wxCommandEvent& event);
	void OnBlackTheme(wxCommandEvent& event);
	void OnScientificNotation(wxCommandEvent& event);

	mpWindow        *m_plot;
	wxTextCtrl      *m_log;

private:
	int axesPos[2];
	mpInfoCoords *nfo; // mpInfoLayer* nfo;
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
	ID_TOGGLE_TICKS,
	ID_TOGGLE_GRID,
	ID_TOGGLE_SCROLLBARS,
	ID_TOGGLE_INFO,
	ID_SAVE_SCREENSHOT,
	ID_TOGGLE_SERIES1,
	ID_TOGGLE_SERIES2,
	ID_TOGGLE_SERIES3,
	ID_BLACK_THEME,
	ID_SCIENTIFIC_NOTATION
};

IMPLEMENT_DYNAMIC_CLASS( MyFrame, wxFrame )

BEGIN_EVENT_TABLE(MyFrame,wxFrame)
	EVT_MENU(ID_ABOUT, MyFrame::OnAbout)
	EVT_MENU(ID_QUIT,  MyFrame::OnQuit)
	EVT_MENU(ID_PRINT_PREVIEW, MyFrame::OnPrintPreview)
	EVT_MENU(ID_PRINT, MyFrame::OnPrint)
	EVT_MENU(mpID_FIT, MyFrame::OnFit)
	EVT_MENU(mpID_ZOOM_IN, MyFrame::OnZoomIn)
	EVT_MENU(mpID_ZOOM_OUT, MyFrame::OnZoomOut)
	EVT_MENU(ID_TOGGLE_TICKS, MyFrame::OnToggleTicks)
	EVT_MENU(ID_TOGGLE_GRID, MyFrame::OnToggleGrid)
	EVT_MENU(ID_TOGGLE_INFO, MyFrame::OnToggleInfoLayer)
	EVT_MENU(ID_SAVE_SCREENSHOT, MyFrame::OnSaveScreenshot)
	EVT_MENU(ID_BLACK_THEME, MyFrame::OnBlackTheme)
	EVT_MENU(ID_SCIENTIFIC_NOTATION, MyFrame::OnScientificNotation)
	EVT_MENU(ID_TOGGLE_SERIES1, MyFrame::OnToggleSeries1)
	EVT_MENU(ID_TOGGLE_SERIES2, MyFrame::OnToggleSeries2)
	EVT_MENU(ID_TOGGLE_SERIES3, MyFrame::OnToggleSeries3)
END_EVENT_TABLE()

MyFrame::MyFrame() : wxFrame( (wxFrame *)NULL, -1, wxT("wxWindows mathplot sample"), wxDefaultPosition, wxSize(1000, 800))
{
	wxMenu *file_menu = new wxMenu();
	wxMenu *view_menu = new wxMenu();
	wxMenu *show_menu = new wxMenu();

	//file_menu->Append( ID_PRINT_PREVIEW, wxT("Print Pre&view..."));
	//file_menu->Append( ID_PRINT, wxT("&Print..."));
	//file_menu->Append( ID_SAVE_SCREENSHOT, wxT("Save screenshot"));
	//file_menu->AppendSeparator();
	file_menu->Append( ID_ABOUT, wxT("&About..."));
	file_menu->Append( ID_QUIT,  wxT("E&xit\tAlt-X"));

	view_menu->Append( mpID_FIT,      wxT("&Fit bounding box"), wxT("Set plot view to show all items"));
	view_menu->Append( mpID_ZOOM_IN,  wxT("Zoom in"),           wxT("Zoom in plot view."));
	view_menu->Append( mpID_ZOOM_OUT, wxT("Zoom out"),          wxT("Zoom out plot view."));
	view_menu->AppendSeparator();
	view_menu->AppendCheckItem( ID_TOGGLE_TICKS, wxT("Show ticks"));    
	view_menu->Check(ID_TOGGLE_TICKS, false);
	view_menu->AppendCheckItem( ID_TOGGLE_GRID, wxT("Show grid"));    
	view_menu->Check(ID_TOGGLE_GRID, true);
	view_menu->AppendCheckItem( ID_TOGGLE_INFO, wxT("Show overlay info box"));
	view_menu->Check(ID_TOGGLE_INFO, true);
	view_menu->AppendCheckItem( ID_BLACK_THEME, wxT("Switch to black background theme"));
	view_menu->AppendCheckItem( ID_SCIENTIFIC_NOTATION, wxT("Show Y axis label as scientific notation"));

	show_menu->AppendCheckItem( ID_TOGGLE_SERIES1, wxT("Series 1"));
	show_menu->AppendCheckItem( ID_TOGGLE_SERIES2, wxT("Series 2"));
	show_menu->AppendCheckItem( ID_TOGGLE_SERIES3, wxT("Series 3"));
	// Start with all plots visible
	show_menu->Check(ID_TOGGLE_SERIES1, true);
	show_menu->Check(ID_TOGGLE_SERIES2, true);
	show_menu->Check(ID_TOGGLE_SERIES3, true);

	wxMenuBar *menu_bar = new wxMenuBar();
	menu_bar->Append(file_menu, wxT("&File"));
	menu_bar->Append(view_menu, wxT("&View"));
	menu_bar->Append(show_menu, wxT("&Show"));

	SetMenuBar( menu_bar );
	CreateStatusBar(1);



	m_plot = new mpWindow( this, -1, wxPoint(0,0), wxSize(100,100), wxSUNKEN_BORDER );
	m_plot->SetColourTheme(*wxWHITE, *wxBLACK, wxColour(96, 96, 96), wxColour(220,220,220));
	//m_plot->SetColourTheme(wxColour(25,25,25), *wxWHITE, grey, wxColour(45,45,45));
	//m_plot->SetColourTheme(*wxWHITE, *wxBLACK, grey, wxColour(220,220,220));
	
	mpScaleX* xaxis = new mpScaleX(wxT("Date"), mpALIGN_BOTTOM, true, mpX_USERDEFINEDDATE);
	mpScaleY* yaxis = new mpScaleY("Value", mpALIGN_LEFT, true);
	yaxis->SetLabelFormat("%1.0f ");
	wxFont graphFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	xaxis->SetFont(graphFont);
	yaxis->SetFont(graphFont);
	xaxis->SetDrawOutsideMargins(false);
	yaxis->SetDrawOutsideMargins(false);
	xaxis->SetTicks(true);
	yaxis->SetTicks(true);
	xaxis->SetGrid(true);
	yaxis->SetGrid(true);
	m_plot->SetMargins(10, 10, 50, 80);
	m_plot->AddLayer(xaxis);
	m_plot->AddLayer(yaxis);
	m_plot->SetTrackBoxXvalueFormat("%b-%Y");

	//m_plot->AddLayer(new mpText(wxT("mpText sample"), 10, 10));
	//wxBrush legendBg(backColor, wxBRUSHSTYLE_SOLID); //SOLID);
	//m_plot->AddLayer( nfo = new mpInfoLayer(wxRect(80,20,40,40), &hatch));
	//mpInfoCoords *nfo;
	//mainPlot->AddLayer(nfo = new mpInfoCoords(wxRect(0, 0, 10, 10), wxTRANSPARENT_BRUSH)); //&hatch));
	//nfo->SetVisible(true);
	//nfo->SetVisible(false);
	//wxBrush hatch2(wxColour(163, 208, 212), wxBRUSHSTYLE_SOLID);

	mpInfoLegend* leg;
	m_plot->AddLayer(leg = new mpInfoLegend(wxRect(96, 20, 40, 40)));
	leg->SetName("legend");
	leg->SetBrush(wxBrush(wxColour(220,220,220), wxBRUSHSTYLE_SOLID));
	leg->SetPen(wxPen(wxColour(25,25,25), 1, wxPENSTYLE_SOLID));
	leg->SetVisible(true);
	
	
	
	std::vector<double>  vectorSeries1, vectorSeries2, vectorSeries3, vectorX;
	std::vector<wxDateTime>  vectorXdate;
		
	wxDateTime yearMonth(01, wxDateTime::Month(4), 2021);
	for (int i = 1; i <= 18; i++) 
	{
		//double i = 5;		
		vectorSeries1.push_back(i);
		vectorSeries2.push_back(i/2);
		vectorSeries3.push_back((18.0-i)/3);
		vectorX.push_back(yearMonth.GetTicks());
		vectorXdate.push_back(yearMonth);
		yearMonth.Add(wxDateSpan(0,1,0));
	}

	wxString series1 = "Series 1";
	wxString series2 = "Series 2";
	wxString series3 = "Series 3";

	mpBAR* vectorLayer1 = new mpBAR(_(series1));
	vectorLayer1->SetData(vectorX, vectorSeries1);
	vectorLayer1->SetContinuity(false);
	wxPen s1pen(*wxGREEN, 6, wxPENSTYLE_SOLID);
	vectorLayer1->SetPen(s1pen);
	vectorLayer1->SetDrawOutsideMargins(true);
	m_plot->AddLayer(vectorLayer1);

	// if (checkChartBalance->GetValue()) {
	mpBAR* vectorLayer2 = new mpBAR(_(series2));
	vectorLayer2->SetData(vectorX, vectorSeries2);
	vectorLayer2->SetContinuity(false);
	wxPen s2Pen(*wxRED, 5, wxPENSTYLE_SOLID);
	vectorLayer2->SetPen(s2Pen);
	//vectorLayer->SetDrawOutsideMargins(true);
	m_plot->AddLayer(vectorLayer2);

	mpBAR* vectorLayer3 = new mpBAR(_(series3));
	vectorLayer3->SetData(vectorX, vectorSeries3);
	vectorLayer3->SetContinuity(false);
	wxPen s3Pen(wxColour(255,120,0), 5, wxPENSTYLE_SOLID);
	vectorLayer3->SetPen(s3Pen);
	//vectorLayer->SetDrawOutsideMargins(true);
	m_plot->AddLayer(vectorLayer3);


	// Create a mpFXYVector layer
	/*mpFXYVector* vectorLayer = new mpFXYVector(_("Vector"));
	vectorLayer->SetTrackBoxYvalueFormat("%s: US$ %.2f");
	// Create two vectors for x,y and fill them with data
	std::vector<double> vectorx, vectory;
	double xcoord;
	for (unsigned int p = 0; p < 100; p++) {
		xcoord = ((double)p-50.0)*5.0;
		vectorx.push_back(xcoord);
		vectory.push_back(0.0001*pow(xcoord, 3));
	}
	vectorLayer->SetData(vectorx, vectory);
	vectorLayer->SetContinuity(true);
	wxPen vectorpen(*wxBLUE, 2, wxPENSTYLE_SOLID);
	vectorLayer->SetPen(vectorpen);
	vectorLayer->SetDrawOutsideMargins(false);
	m_plot->AddLayer(     vectorLayer );


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

	m_plot->EnableDoubleBuffer(true);*/
	
	m_plot->BindMouseButton(mpDOUBLE_CLICK, mpFIT);
	m_plot->BindMouseButton(mpMIDDLE_DOWN, mpPAN);
	//m_plot->BindMouseButton(mpRIGHT_DOWN, mpCONTEXT_MENU);
	m_plot->BindMouseButton(mpRIGHT_DOWN, mpTRACK);
	m_plot->BindMouseButton(mpLEFT_DOWN, mpZOOM_RECTANGLE);

	m_plot->BindMouseWheel(mpWHEEL, mpZOOM);
	m_plot->BindMouseWheel(mpSHIFT_WHEEL, mpHORIZONTAL_PAN);
	m_plot->BindMouseWheel(mpCTRL_WHEEL, mpVERTICAL_PAN);

	m_plot->SetTrackBoxYvalueFormat("%s: %.3f");

	m_plot->Fit();
}

void MyFrame::OnQuit( wxCommandEvent &WXUNUSED(event) )
{
	Close( TRUE );
}

void MyFrame::OnFit( wxCommandEvent &WXUNUSED(event) )
{
	m_plot->Fit();
}

void MyFrame::OnZoomIn( wxCommandEvent &WXUNUSED(event) )
{
	m_plot->ZoomIn();
}

void MyFrame::OnZoomOut( wxCommandEvent &WXUNUSED(event) )
{
	m_plot->ZoomOut();
}


void MyFrame::OnAbout( wxCommandEvent &WXUNUSED(event) )
{
	wxMessageBox( wxT("wxWidgets mathplot sample\n(c) 2003 David Schalig\n(c) 2007-2009 Davide Rondini and wxMathPlot team\n       2022 Naiche Barcelos"));
}

void MyFrame::OnToggleTicks( wxCommandEvent& event)
{
	((mpScaleX*)(m_plot->GetLayer(0)))->SetTicks(event.IsChecked());
	((mpScaleY*)(m_plot->GetLayer(1)))->SetTicks(event.IsChecked());
	m_plot->UpdateAll();
}

void MyFrame::OnToggleGrid( wxCommandEvent& event)
{
	((mpScaleX*)(m_plot->GetLayer(0)))->SetGrid(event.IsChecked());
	((mpScaleY*)(m_plot->GetLayer(1)))->SetGrid(event.IsChecked());
	m_plot->UpdateAll();
}

void MyFrame::OnToggleInfoLayer(wxCommandEvent& event)
{
	if (event.IsChecked())
		nfo->SetVisible(true);
	else
		nfo->SetVisible(false);
	m_plot->UpdateAll();
	event.Skip();
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

void MyFrame::OnScientificNotation(wxCommandEvent& event)
{
	mpScaleY* yaxis = ((mpScaleY*)(m_plot->GetLayer(1)));
	if (event.IsChecked()){	
		yaxis->SetLabelFormat("%1.1e ");
	}
	else
	{
		yaxis->SetLabelFormat("%1.0f ");
	}
	m_plot->UpdateAll();
}

void MyFrame::OnPrintPreview( wxCommandEvent &WXUNUSED(event))
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

void MyFrame::OnPrint( wxCommandEvent& WXUNUSED(event) )
{
	wxPrinter printer;
	mpPrintout printout(m_plot, wxT("Plot print"));
	printer.Print(this, &printout, true);
}

void MyFrame::OnSaveScreenshot(wxCommandEvent& event)
{
	wxFileDialog fileDialog(this, _("Save a screenshot"), wxT(""), wxT(""), wxT("BMP image (*.bmp) | *.bmp|JPEG image (*.jpg) | *.jpeg;*.jpg|PNG image (*.png) | *.png"), wxFD_SAVE);
	if(fileDialog.ShowModal() == wxID_OK) {
		wxFileName namePath(fileDialog.GetPath());
		int fileType = wxBITMAP_TYPE_BMP;
		if( namePath.GetExt().CmpNoCase(wxT("jpeg")) == 0 ) fileType = wxBITMAP_TYPE_JPEG;
		if( namePath.GetExt().CmpNoCase(wxT("jpg")) == 0 )  fileType = wxBITMAP_TYPE_JPEG;
		if( namePath.GetExt().CmpNoCase(wxT("png")) == 0 )  fileType = wxBITMAP_TYPE_PNG;
		wxSize imgSize(500,500);
		m_plot->SaveScreenshot(fileDialog.GetPath(), fileType, imgSize, false);
	}
	event.Skip();
}

void MyFrame::OnToggleSeries1(wxCommandEvent& event)
{
	m_plot->SetLayerVisible(wxT("Series 1"), event.IsChecked());
}

void MyFrame::OnToggleSeries2(wxCommandEvent& event)
{
	m_plot->SetLayerVisible(wxT("Series 2"), event.IsChecked());
}

void MyFrame::OnToggleSeries3(wxCommandEvent& event)
{
	m_plot->SetLayerVisible(wxT("Series 3"), event.IsChecked());
}

//-----------------------------------------------------------------------------
// MyApp
//-----------------------------------------------------------------------------

bool MyApp::OnInit()
{
	wxInitAllImageHandlers();
	wxFrame *frame = new MyFrame();
	frame->Show( TRUE );

	return TRUE;
}
