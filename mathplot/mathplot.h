/////////////////////////////////////////////////////////////////////////////
// Name:        mathplot.h
// Purpose:     Framework for mathematical graph plotting in wxWindows
// Author:      David Schalig
// Modified by:
// Created:     21/07/2003
// Copyright:   (c) David Schalig
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _MP_MATHPLOT_H_
#define _MP_MATHPLOT_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "mathplot.h"
#endif

#include "wx/defs.h"

#include "wx/scrolwin.h"
#include "wx/event.h"
#include "wx/dynarray.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class WXDLLEXPORT mpLayer;
class WXDLLEXPORT mpFX;
class WXDLLEXPORT mpFY;
class WXDLLEXPORT mpFXY;
class WXDLLEXPORT mpScaleX;
class WXDLLEXPORT mpScaleY;
class WXDLLEXPORT mpWindow;

enum
{
    mpID_FIT = 2000,
    mpID_ZOOM_IN,
    mpID_ZOOM_OUT,
    mpID_CENTER,
    mpID_LOCKASPECT,
};

//-----------------------------------------------------------------------------
// mpView
//-----------------------------------------------------------------------------

struct WXDLLEXPORT mpView
{
};

//-----------------------------------------------------------------------------
// mpLayer
//-----------------------------------------------------------------------------

class WXDLLEXPORT mpLayer : public wxObject
{
public:
    mpLayer();

    virtual double GetMinX() { return -1.0; }
    virtual double GetMaxX() { return  1.0; }
    virtual double GetMinY() { return -1.0; }
    virtual double GetMaxY() { return  1.0; }
    virtual void   Plot(wxDC & dc, mpWindow & w) = 0;

    wxString       GetName() const { return m_name; }
    const wxFont&  GetFont() const { return m_font; }
    const wxPen&   GetPen()  const { return m_pen;  }

    void SetName(wxString name) { m_name = name; }
    void SetFont(wxFont& font)  { m_font = font; }
    void SetPen(wxPen& pen)     { m_pen  = pen;  }

protected:
    wxFont   m_font;
    wxPen    m_pen;
    wxString m_name;

    DECLARE_CLASS(mpLayer)
};

//-----------------------------------------------------------------------------
// mpLayer implementations - functions
//-----------------------------------------------------------------------------

#define mpALIGNMASK    0x03
#define mpALIGN_RIGHT  0x00
#define mpALIGN_CENTER 0x01
#define mpALIGN_LEFT   0x02
#define mpALIGN_TOP    mpALIGN_RIGHT
#define mpALIGN_BOTTOM mpALIGN_LEFT
#define mpALIGN_NE     0x00
#define mpALIGN_NW     0x01
#define mpALIGN_SW     0x02
#define mpALIGN_SE     0x03

class WXDLLEXPORT mpFX : public mpLayer
{
public:
    mpFX(wxString name = wxEmptyString, int flags = mpALIGN_RIGHT);

    virtual double GetY( double x ) { return x; }
    virtual void Plot(wxDC & dc, mpWindow & w);

protected:
    int m_flags;

    DECLARE_CLASS(mpFX)
};

class WXDLLEXPORT mpFY : public mpLayer
{
public:
    mpFY(wxString name = wxEmptyString, int flags = mpALIGN_TOP);

    virtual double GetX( double y ) { return y; }
    virtual void Plot(wxDC & dc, mpWindow & w);

protected:
    int m_flags;

    DECLARE_CLASS(mpFY)
};

class WXDLLEXPORT mpFXY : public mpLayer
{
public:
    mpFXY(wxString name = wxEmptyString, int flags = mpALIGN_NE);

    virtual int  GetNumSamples() { return 0; }
    virtual void GetXY( int n, double & x, double & y ) {}
    virtual void Plot(wxDC & dc, mpWindow & w);

protected:
    int m_flags;

    DECLARE_CLASS(mpFXY)
};

//-----------------------------------------------------------------------------
// mpLayer implementations - furniture (scales, ...)
//-----------------------------------------------------------------------------

class WXDLLEXPORT mpScaleX : public mpLayer
{
public:
    mpScaleX(wxString name = wxT("X"));
    virtual void Plot(wxDC & dc, mpWindow & w);
    
    DECLARE_CLASS(mpScaleX)
};

class WXDLLEXPORT mpScaleY : public mpLayer
{
public:
    mpScaleY(wxString name = wxT("Y"));
    virtual void Plot(wxDC & dc, mpWindow & w);

protected:

    DECLARE_CLASS(mpScaleY)
};

//-----------------------------------------------------------------------------
// mpWindow
//-----------------------------------------------------------------------------

#define mpMOUSEMODE_DRAG    0
#define mpMOUSEMODE_ZOOMBOX 1

class WXDLLEXPORT mpWindow : public wxScrolledWindow
{
public:
    mpWindow() {}
    mpWindow( wxWindow *parent, wxWindowID id,
                     const wxPoint &pos = wxDefaultPosition, 
                     const wxSize &size = wxDefaultSize,
                     int flags = 0);
    ~mpWindow();

    void AddLayer( mpLayer* layer);
    void DelLayer( mpLayer* layer);

    double GetScaleX(void) const { return m_scaleX; }
    double GetScaleY(void) const { return m_scaleY; }
    double GetPosX(void) const { return m_posX; }
    double GetPosY(void) const { return m_posY; }
    int    GetScrX(void) const { return m_scrX; }
    int    GetScrY(void) const { return m_scrY; }

    void SetScaleX(double scaleX) { if (scaleX!=0) m_scaleX=scaleX; UpdateAll(); }
    void SetScaleY(double scaleY) { if (scaleY!=0) m_scaleY=scaleY; UpdateAll(); }
    void SetPosX(double posX) { m_posX=posX; UpdateAll(); }
    void SetPosY(double posY) { m_posY=posY; UpdateAll(); }
    void SetPos( double posX, double posY) { m_posX=posX; m_posY=posY; UpdateAll(); }

    void LockAspect(bool enable = TRUE);

    void Fit();
    void ZoomIn();
    void ZoomOut();

    void UpdateAll();

protected:
    void OnPaint        (wxPaintEvent   &event);
    void OnSize         (wxSizeEvent    &event);
    void OnScroll2(wxScrollWinEvent &event);
    void OnShowPopupMenu(wxMouseEvent   &event);
    void OnCenter       (wxCommandEvent &event);
    void OnFit          (wxCommandEvent &event);
    void OnZoomIn       (wxCommandEvent &event);
    void OnZoomOut      (wxCommandEvent &event);
    void OnLockAspect   (wxCommandEvent &event);

    bool UpdateBBox();

    wxList m_layers;
    wxMenu m_popmenu;
    bool  m_lockaspect;

    double m_minX;
    double m_maxX;
    double m_minY;
    double m_maxY;
    double m_scaleX;
    double m_scaleY;
    double m_posX;
    double m_posY;
    int    m_scrX;
    int    m_scrY;
    int    m_clickedX;
    int    m_clickedY;

    DECLARE_CLASS(mpWindow)
    DECLARE_EVENT_TABLE()
};

#endif // _MP_MATHPLOT_H_
