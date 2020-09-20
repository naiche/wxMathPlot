This fork's purpose is to make available any enhancements/fixes made when using this code.

TODO:
  * Fix error when zooming out or panning too far (mpX_DATE, mpX_DATETIME or mpX_TIME)
  * Use wxDateTime instead of time_t

DONE:
  * Changeable grid color
        SetColourTheme(backgroundColour, foregroundColour, axesColour, gridColour);
  * Configurable mouse commands
            m_plot->BindMouseButton(command*, function**)
                * -> command can be mpDOUBLE_CLICK, mpMIDDLE_DOWN, mpRIGHT_DOWN or mpLEFT_DOWN.
                ** -> function can be mpFIT, mpPAN, mpCONTEXT_MENU, mpTRACK or mpZOOM_RECTANGLE.
	        
	        m_plot->BindMouseWheel(command*, function**);
                * -> command can be mpWHEEL, mpSHIFT_WHEEL or mpCTRL_WHEEL.
                ** -> function can be mpZOOM, mpHORIZONTAL_PAN or mpVERTICAL_PAN.

  * Displays the coordinates of the nearest datapoint on mpXYVector, mpFX, mpFY and mpFXY layers  (mpTrack)
  * Disappearing zoom rectangle fixed
