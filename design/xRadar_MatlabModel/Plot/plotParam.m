% 
% Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
% 

function paramPlot = plotParam ()

%% Author B. Berkane
%  Plotting Parameters.  

%% Error reporting mode
% errPlot := Matlab vs Ai-Engine error plot type
% errPlot = "s2S"    := Plot absolute error vector between single-precison
%                       Matlab and single-precison AiEngine.

% errPlot = "roundedS2S" := Same as above but round the ADPCA output vectors
%                           to a number of significant Bits as set by the
%                           significantBits before calculating the
%                           error
enableTcpRxListener = true;
figurePos           = [10 10 1400  700];
errPlot             = "roundedS2S";
significantBits     = 7;
nFFT                = 256;

paramPlot.enableTcpRxListener = enableTcpRxListener;
paramPlot.figurePos           = figurePos;
paramPlot.errPlot             = errPlot ;
paramPlot.significantBits     = significantBits ;
paramPlot.nFFT                = nFFT;     
 
    
end