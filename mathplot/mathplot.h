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

/** @file mathplot.h */
/** @mainpage wxMathPlot
    wxMathPlot is a framework for mathematical graph plotting in wxWindows.

    The framework is designed for convenience and ease of use.

    @section overview Overview
    The heart of wxMathPlot is mpWindow, which is a 2D canvas for plot layers.
    mpWindow can be embedded as subwindow in a wxPane, a wxFrame, or any other wxWindow.
    mpWindow provides a zoomable and moveable view of the layers. The current view can
    be controlled with the mouse, the scrollbars, and a context menu.

    Plot layers are implementations of the abstract base class mpLayer. Those can
    be function plots, scale rulers, or any other vector data visualisation. wxMathPlot provides
    two mpLayer implementations for plotting horizontal and vertical rulers: mpScaleX and mpScaleY.
    For convenient function plotting three more abstract base classes derived from mpLayer
    are provided: mpFX, mpFY and mpFXY. These base classes already come with plot code, own
    functions can be implemented by overiding just one member for retrieving a function value.

    @section coding Coding conventions
    wxMathPlot sticks to wxWindow's coding conventions. All entities defined by wxMathPlot
    have the prefix <i>mp</i>.

    @section author Author and license
    wxMathPlot is published under the terms of the wxWindow license.
    The author David Schalig can be contacted via the wxMathPlot's homepage at
    http://sourceforge.net/projects/wxmathplot
*/

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

/** Command IDs used by mpWindow */
enum
{
    mpID_FIT = 2000,    //!< Fit view to match bounding box of all layers
    mpID_ZOOM_IN,       //!< Zoom into view at clickposition / window center
    mpID_ZOOM_OUT,      //!< Zoom out
    mpID_CENTER,        //!< Center view on click position
    mpID_LOCKASPECT,    //!< Lock x/y scaling aspect
};

//-----------------------------------------------------------------------------
// mpLayer
//-----------------------------------------------------------------------------

/** Plot layer, abstract base class.
    Any number of mpLayer implementations can be attached to mpWindow.
    Examples for mpLayer implementations are function graphs, or scale rulers.

    For convenience mpLayer defines a name, a font (wxFont), and a pen (wxPen)
    as class members. These may or may not be used by implementations.
*/
class WXDLLEXPORT mpLayer : public wxObject
{
public:
    mpLayer();

    /** Check whether this layer has a bounding box.
        The default implementation returns \a TRUE. Overide and return
        FALSE if your mpLayer implementation should be ignored by the calculation
        of the global bounding box for all layers in a mpWindow.
        @retval TRUE Has bounding box
        @retval FALSE Has not bounding box
    */
    virtual bool   HasBBox() { return TRUE; }

    /** Get inclusive left border of bounding box.
        @return Value
    */
    virtual double GetMinX() { return -1.0; }

    /** Get inclusive right border of bounding box.
        @return Value
    */
    virtual double GetMaxX() { return  1.0; }

    /** Get inclusive bottom border of bounding box.
        @return Value
    */
    virtual double GetMinY() { return -1.0; }

    /** Get inclusive top border of bounding box.
        @return Value
    */
    virtual double GetMaxY() { return  1.0; }

    /** Plot given view of layer to the given device context.
        An implementation of this function has to transform layer coordinates to
        wxDC coordinates based on the view parameters retrievable from the mpWindow
        passed in \a w. The passed device context \a dc has its coordinate origin set
        to the center of the visible area. The coordinate orientation is as show in the
        following picture:
        <pre>
        +--------------------------------------------------+
        |                                                  |
        |                                                  |
        |                (wxDC origin 0,0)                 |
        |                       x-------------> acending X |
        |                       |                          |
        |                       |                          |
        |                       V ascending Y              |
        +--------------------------------------------------+
        </pre>
        Note that Y ascends in downward direction, whereas the usual vertical orientation
        for mathematical plots is vice versa. Thus Y-orientation will be swapped usually,
        when transforming between wxDC and mpLayer coordinates.

        <b> Rules for transformation between mpLayer and wxDC coordinates </b>
        @code
        dc_X = (layer_X - mpWindow::GetPosX()) * mpWindow::GetScaleX()
        dc_Y = (mpWindow::GetPosY() - layer_Y) * mpWindow::GetScaleY() // swapping Y-orientation

        layer_X = (dc_X / mpWindow::GetScaleX()) + mpWindow::GetPosX() // scale guaranted to be not 0
        layer_Y = mpWindow::GetPosY() - (dc_Y / mpWindow::GetScaleY()) // swapping Y-orientation
        @endcode

        @param dc Device context to plot to.
        @param w  View to plot. The visible area can be retrieved from this object.
    */
    virtual void   Plot(wxDC & dc, mpWindow & w) = 0;

    /** Get layer name.
        @return Name
    */
    wxString       GetName() const { return m_name; }

    /** Get font set for this layer.
        @return Font
    */
    const wxFont&  GetFont() const { return m_font; }

    /** Get pen set for this layer.
        @return Pen
    */
    const wxPen&   GetPen()  const { return m_pen;  }

    /** Set layer name
        @param name Name, will be copied to internal class member
    */
    void SetName(wxString name) { m_name = name; }

    /** Set layer font
        @param font Font, will be copied to internal class member
    */
    void SetFont(wxFont& font)  { m_font = font; }

    /** Set layer pen
        @param pen Pen, will be copied to internal class member
    */
    void SetPen(wxPen& pen)     { m_pen  = pen;  }

protected:
    wxFont   m_font;    //!< Layer's font
    wxPen    m_pen;     //!< Layer's pen
    wxString m_name;    //!< Layer's name

    DECLARE_CLASS(mpLayer)
};

//-----------------------------------------------------------------------------
// mpLayer implementations - functions
//-----------------------------------------------------------------------------

/** @name Label alignment constants
@{*/

/** @internal */
#define mpALIGNMASK    0x03
/** Aligns label to the right. For use with mpFX. */
#define mpALIGN_RIGHT  0x00
/** Aligns label to the center. For use with mpFX and mpFY. */
#define mpALIGN_CENTER 0x01
/** Aligns label to the left. For use with mpFX. */
#define mpALIGN_LEFT   0x02
/** Aligns label to the top. For use with mpFY. */
#define mpALIGN_TOP    mpALIGN_RIGHT
/** Aligns label to the bottom. For use with mpFY. */
#define mpALIGN_BOTTOM mpALIGN_LEFT
/** Aligns label to north-east. For use with mpFXY. */
#define mpALIGN_NE     0x00
/** Aligns label to north-west. For use with mpFXY. */
#define mpALIGN_NW     0x01
/** Aligns label to south-west. For use with mpFXY. */
#define mpALIGN_SW     0x02
/** Aligns label to south-east. For use with mpFXY. */
#define mpALIGN_SE     0x03

/*@}*/

/** @name mpLayer implementations - functions
@{*/

/** Abstract base class providing plot and labeling functionality for functions F:X->Y.
    Override mpFX::GetY to implement a function.
    Optionally implement a constructor and pass a name (label) and a label alignment
    to the constructor mpFX::mpFX. If the layer name is empty, no label will be plotted.
*/
class WXDLLEXPORT mpFX : public mpLayer
{
public:
    /** @param name  Label
        @param flags Label alignment, pass one of #mpALIGN_RIGHT, #mpALIGN_CENTER, #mpALIGN_LEFT.
    */
    mpFX(wxString name = wxEmptyString, int flags = mpALIGN_RIGHT);

    /** Get function value for argument.
        Override this function in your implementation.
        @param x Argument
        @return Function value
    */
    virtual double GetY( double x ) = 0;

    /** Layer plot handler.
        This implementation will plot the function in the visible area and
        put a label according to the aligment specified.
    */
    virtual void Plot(wxDC & dc, mpWindow & w);

protected:
    int m_flags; //!< Holds label alignment

    DECLARE_CLASS(mpFX)
};

/** Abstract base class providing plot and labeling functionality for functions F:Y->X.
    Override mpFY::GetX to implement a function.
    Optionally implement a constructor and pass a name (label) and a label alignment
    to the constructor mpFY::mpFY. If the layer name is empty, no label will be plotted.
*/
class WXDLLEXPORT mpFY : public mpLayer
{
public:
    /** @param name  Label
        @param flags Label alignment, pass one of #mpALIGN_BOTTOM, #mpALIGN_CENTER, #mpALIGN_TOP.
    */
    mpFY(wxString name = wxEmptyString, int flags = mpALIGN_TOP);

    /** Get function value for argument.
        Override this function in your implementation.
        @param y Argument
        @return Function value
    */
    virtual double GetX( double y ) = 0;

    /** Layer plot handler.
        This implementation will plot the function in the visible area and
        put a label according to the aligment specified.
    */
    virtual void Plot(wxDC & dc, mpWindow & w);

protected:
    int m_flags; //!< Holds label alignment

    DECLARE_CLASS(mpFY)
};

/** Abstract base class providing plot and labeling functionality for a locus plot F:N->X,Y.
    Locus argument N is assumed to be in range 0 .. MAX_N, and implicitely derived by enumrating
    all locus values. Override mpFXY::Rewind and mpFXY::GetNextXY to implement a locus.
    Optionally implement a constructor and pass a name (label) and a label alignment
    to the constructor mpFXY::mpFXY. If the layer name is empty, no label will be plotted.
*/
class WXDLLEXPORT mpFXY : public mpLayer
{
public:
    /** @param name  Label
        @param flags Label alignment, pass one of #mpALIGN_NE, #mpALIGN_NW, #mpALIGN_SW, #mpALIGN_SE.
    */
    mpFXY(wxString name = wxEmptyString, int flags = mpALIGN_NE);

    /** Rewind value enumeration with mpFXY::GetNextXY.
        Override this function in your implementation.
    */
    virtual void Rewind() = 0;

    /** Get locus value for next N.
        Override this function in your implementation.
        @param x Returns X value
        @param y Returns Y value
    */
    virtual bool GetNextXY(double & x, double & y) = 0;

    /** Layer plot handler.
        This implementation will plot the locus in the visible area and
        put a label according to the aligment specified.
    */
    virtual void Plot(wxDC & dc, mpWindow & w);

protected:
    int m_flags; //!< Holds label alignment

    DECLARE_CLASS(mpFXY)
};

/*@}*/

//-----------------------------------------------------------------------------
// mpLayer implementations - furniture (scales, ...)
//-----------------------------------------------------------------------------

/** @name mpLayer implementations - furniture (scales, ...)
@{*/

/** Plot layer implementing a x-scale ruler.
    The ruler is fixed at Y=0 in the coordinate system. A label is plottet at
    the bottom-right hand of the ruler. The scale numbering automatically
    adjusts to view and zoom factor.
*/
class WXDLLEXPORT mpScaleX : public mpLayer
{
public:
    /** @param name Label to plot by the ruler */
    mpScaleX(wxString name = wxT("X"));

    /** Layer plot handler.
        This implementation will plot the ruler adjusted to the visible area.
    */
    virtual void Plot(wxDC & dc, mpWindow & w);

    /** Check whether this layer has a bounding box.
        This implementation returns \a FALSE thus making the ruler invisible
        to the plot layer bounding box calculation by mpWindow.
    */
    virtual bool HasBBox() { return FALSE; }
    
    DECLARE_CLASS(mpScaleX)
};

/** Plot layer implementing a y-scale ruler.
    The ruler is fixed at X=0 in the coordinate system. A label is plottet at
    the top-right hand of the ruler. The scale numbering automatically
    adjusts to view and zoom factor.
*/
class WXDLLEXPORT mpScaleY : public mpLayer
{
public:
    /** @param name Label to plot by the ruler */
    mpScaleY(wxString name = wxT("Y"));

    /** Layer plot handler.
        This implementation will plot the ruler adjusted to the visible area.
    */
    virtual void Plot(wxDC & dc, mpWindow & w);

    /** Check whether this layer has a bounding box.
        This implementation returns \a FALSE thus making the ruler invisible
        to the plot layer bounding box calculation by mpWindow.
    */
    virtual bool HasBBox() { return FALSE; }

protected:

    DECLARE_CLASS(mpScaleY)
};

//-----------------------------------------------------------------------------
// mpWindow
//-----------------------------------------------------------------------------

/** @name Constants defining mouse modes for mpWindow
@{*/

/** Mouse panning drags the view. Mouse mode for mpWindow. */
#define mpMOUSEMODE_DRAG    0
/** Mouse panning creates a zoom box. Mouse mode for mpWindow. */
#define mpMOUSEMODE_ZOOMBOX 1

/*@}*/

/** Canvas for plotting mpLayer implementations.

    This class defines a zoomable and moveable 2D plot canvas. Any number
    of mpLayer implementations (scale rulers, function plots, ...) can be
    attached using mpWindow::AddLayer.

    The canvas window provides a context menu with actions for navigating the view.
    The context menu can be retrieved with mpWindow::GetPopupMenu, e.g. for extending it
    externally.
*/
class WXDLLEXPORT mpWindow : public wxScrolledWindow
{
public:
    mpWindow() {}
    mpWindow( wxWindow *parent, wxWindowID id,
                     const wxPoint &pos = wxDefaultPosition, 
                     const wxSize &size = wxDefaultSize,
                     int flags = 0);
    ~mpWindow();

    /** Get reference to context menu of the plot canvas.
        @return Pointer to menu. The menu can be modified.
    */
    wxMenu* GetPopupMenu() { return &m_popmenu; }

    /** Add a plot layer to the canvas.
        @param layer Pointer to layer. The mpLayer object will get under control of mpWindow,
                     i.e. it will be delete'd on mpWindow destruction
        @retval TRUE Success
        @retval FALSE Failure due to out of memory.
    */
    bool AddLayer( mpLayer* layer);

    /** Remove a plot layer from the canvas.
        @param layer Pointer to layer. The mpLayer object will be destructed using delete.
    */
    void DelLayer( mpLayer* layer);

    /** Get current view's X scale.
        See @ref mpLayer::Plot "rules for coordinate transformation"
        @return Scale
    */
    double GetScaleX(void) const { return m_scaleX; }

    /** Get current view's Y scale.
        See @ref mpLayer::Plot "rules for coordinate transformation"
        @return Scale
    */
    double GetScaleY(void) const { return m_scaleY; }

    /** Get current view's X position.
        See @ref mpLayer::Plot "rules for coordinate transformation"
        @return X Position in layer coordinate system, that corresponds to the center point of the view.
    */
    double GetPosX(void) const { return m_posX; }

    /** Get current view's Y position.
        See @ref mpLayer::Plot "rules for coordinate transformation"
        @return Y Position in layer coordinate system, that corresponds to the center point of the view.
    */
    double GetPosY(void) const { return m_posY; }

    /** Get current view's X dimension in device context units.
        Usually this is equal to wxDC::GetSize, but it might differ thus mpLayer
        implementations should rely on the value returned by the function.
        See @ref mpLayer::Plot "rules for coordinate transformation"
        @return X dimension. 
    */
    int GetScrX(void) const { return m_scrX; }

    /** Get current view's Y dimension in device context units.
        Usually this is equal to wxDC::GetSize, but it might differ thus mpLayer
        implementations should rely on the value returned by the function.
        See @ref mpLayer::Plot "rules for coordinate transformation"
        @return Y dimension. 
    */
    int GetScrY(void) const { return m_scrY; }

    /** Set current view's X scale and refresh display. 
        @param scaleX New scale, must not be 0.
    */
    void SetScaleX(double scaleX) { if (scaleX!=0) m_scaleX=scaleX; UpdateAll(); }

    /** Set current view's Y scale and refresh display. 
        @param scaleY New scale, must not be 0.
    */
    void SetScaleY(double scaleY) { if (scaleY!=0) m_scaleY=scaleY; UpdateAll(); }

    /** Set current view's X position and refresh display. 
        @param posX New position that corresponds to the center point of the view.
    */
    void SetPosX(double posX) { m_posX=posX; UpdateAll(); }

    /** Set current view's Y position and refresh display. 
        @param posY New position that corresponds to the center point of the view.
    */
    void SetPosY(double posY) { m_posY=posY; UpdateAll(); }

    /** Set current view's X and Y position and refresh display. 
        @param posX New position that corresponds to the center point of the view.
        @param posY New position that corresponds to the center point of the view.
    */
    void SetPos( double posX, double posY) { m_posX=posX; m_posY=posY; UpdateAll(); }

    /** Enable or disable X/Y scale aspect locking for the view.
        @note Explicit calls to mpWindow::SetScaleX and mpWindow::SetScaleY will set
              an unlocked apect, but any other action changing the view scale will
              lock the aspect again.
    */
    void LockAspect(bool enable = TRUE);

    /** Checks whether the X/Y scale aspect is locked.
        @retval TRUE Locked
        @retval FALSE Unlocked
    */
    inline bool IsAspectLocked() { return m_lockaspect; }

    /** Set view to fit global bounding box of all plot layers and refresh display.
        Scale and position will be set to a show all attached mpLayers.
        The X/Y scale aspect lock is taken into account.
    */
    void Fit();

    /** Zoom into current view and refresh display */
    void ZoomIn();

    /** Zoom out current view and refresh display */
    void ZoomOut();

    /** Refresh display */
    void UpdateAll();

protected:
    void OnPaint         (wxPaintEvent     &event); //!< Paint handler, will plot all attached layers
    void OnSize          (wxSizeEvent      &event); //!< Size handler, will update scroll bar sizes
    void OnScroll2       (wxScrollWinEvent &event); //!< Scroll handler, will move canvas
    void OnShowPopupMenu (wxMouseEvent     &event); //!< Mouse handler, will show context menu
    void OnCenter        (wxCommandEvent   &event); //!< Context menu handler
    void OnFit           (wxCommandEvent   &event); //!< Context menu handler
    void OnZoomIn        (wxCommandEvent   &event); //!< Context menu handler
    void OnZoomOut       (wxCommandEvent   &event); //!< Context menu handler
    void OnLockAspect    (wxCommandEvent   &event); //!< Context menu handler

    bool UpdateBBox(); //!< Recalculate global layer bounding box

    wxList m_layers;    //!< List of attached plot layers
    wxMenu m_popmenu;   //!< Canvas' context menu
    bool   m_lockaspect;//!< Scale aspect is locked or not

    double m_minX;      //!< Global layer bounding box, left border incl.
    double m_maxX;      //!< Global layer bounding box, right border incl.
    double m_minY;      //!< Global layer bounding box, bottom border incl.
    double m_maxY;      //!< Global layer bounding box, top border incl.
    double m_scaleX;    //!< Current view's X scale
    double m_scaleY;    //!< Current view's Y scale
    double m_posX;      //!< Current view's X position
    double m_posY;      //!< Current view's Y position
    int    m_scrX;      //!< Current view's X dimension
    int    m_scrY;      //!< Current view's Y dimension
    int    m_clickedX;  //!< Last mouse click X position, for centering and zooming the view
    int    m_clickedY;  //!< Last mouse click Y position, for centering and zooming the view

    DECLARE_CLASS(mpWindow)
    DECLARE_EVENT_TABLE()
};

#endif // _MP_MATHPLOT_H_
