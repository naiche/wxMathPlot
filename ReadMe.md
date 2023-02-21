This project is based on the version of wxMathPlot found at MachMotionOpenSource/wxMathPlot and this repository's purpose is to make available any enhancements/fixes made when using this code.


Sample1 -> mpXYVector, mpFX, mpFY and mpFXY example
Sample2 -> Comparison of Integer Coding Algorithms
Sample3 -> mpMovableObject example
Sample4 -> Bar chart example (mpBAR)


DONE:
  
  * Configurable mouse commands
            
	    mpWindow->BindMouseButton(command*, function**);
                * -> command can be mpDOUBLE_CLICK, mpMIDDLE_DOWN, mpRIGHT_DOWN or mpLEFT_DOWN.
                ** -> function can be mpFIT, mpPAN, mpCONTEXT_MENU, mpTRACK or mpZOOM_RECTANGLE.
	        
	    mpWindow->BindMouseWheel(command*, function**);
                * -> command can be mpWHEEL, mpSHIFT_WHEEL or mpCTRL_WHEEL.
                ** -> function can be mpZOOM, mpHORIZONTAL_PAN or mpVERTICAL_PAN.


  * mpTrack => Displays the coordinates of the nearest datapoint on mpXYVector, mpFX, mpFY and mpFXY layers
  
   	<img src="/mathplot/images/Light.png" alt="mpTrack" width="75%" height="75%" align="middle">
 
 
  * Changeable grid color
        	
		mpWindow::SetColourTheme(backgroundColour, foregroundColour, axesColour, gridColour);
		
  	 <img src="/mathplot/images/Dark.png" alt="DarkThemed" width="75%" height="75%" align="middle">
 
  

 * Format of X value in the TrackBox can be set when X axis is mpX_USERDEFINED or mpX_USERDEFINEDDATE
  	
		mpWindow::SetTrackBoxXvalueFormat(wxString);
		
 
 * Format of Y value in the TrackBox can be set individually 
  	
		mpLayer::SetTrackBoxYvalueFormat(wxString);
	
    or collectively 
	
		mpWindow::SetTrackBoxYvalueFormat(wxString);
  
  * Disappearing zoom rectangle fixed
  * Using wxDateTime instead of time_t
  * Fixed error when zooming out or panning too far (mpX_DATE, mpX_DATETIME or mpX_TIME)
  * Fixed bug where mpInfoLayer->SetBrush()  didn’t apply changes to wxBrush. 
  
  
  
TO DO:
  
  - Improve Bar Chart functionality
