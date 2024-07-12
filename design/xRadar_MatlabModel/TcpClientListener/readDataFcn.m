% 
% Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
% 

function readDataFcn(src, ~)
%% Author Bachir B./Fidus

% This function calls the following functions:
%  - ./TcpClientListner/getGlobal      (Fidus)
%  - ./Plot/plotAngDopResp  (Fidus)
%  - read (TCP/IP Object function, Instrumentation-Ctrl Toolbox)
%  - ./Radar/xRadarParam    (Fidus)
%  - ./ADPCA/adpcaCanceller (Fidus)

%% Add Paths
addpath ./ADPCA/ ./PLot/  ./Radar/ ./TcpClientListener/
%% Get Golbal varaiables: Target Range Index and Matlab-calculated Z vector
[xC, paramSTAP, rxHeaderNbytes] = getGlobalVar;


%% Read pkt when received and build the z and w vectors
fprintf('\n -- <strong>NOTE</strong>: Received %d Bytes from the TCP server \n', src.NumBytesAvailable);
rxDat  = read(src,src.NumBytesAvailable/4,"single");

% Cuboid Serial number;
sNbr = rxDat(2);

N               = paramSTAP(sNbr).radarAntenna.NumElements ;
M               = paramSTAP(sNbr).numPulseInt;
L               = paramSTAP(sNbr).samplesPerFrame;
targetRange     = paramSTAP(sNbr).target1Range;
rangeGateWidth  = paramSTAP(sNbr).rangeGateWidth;
maxRange        = paramSTAP(sNbr).maxRange;
errPlot         = paramSTAP(sNbr).errPlot;
significantBits = paramSTAP(sNbr).significantBits;

% Extract the w and z vectors from the received bytes
zIQ   = rxDat(rxHeaderNbytes/4+1:end);
z     = single(zIQ(1:2:end)+ 1j * zIQ(2:2:end));

nshift = -26;
z = circshift(z,nshift);
z(end+nshift+1:end) = 0;

zNorm  = single(abs(z)./max(abs(z))); % Normalized
%% Run reference ADPCA
fprintf("\n -- <strong>NOTE</strong>: Running Reference ADPCA Canceller on Cuboid #%d \n", sNbr);
xC = squeeze(xC(:,:,:,sNbr));
adpcaMetrics = adpcaCanceller (single(xC), paramSTAP(sNbr));
wRef = single(adpcaMetrics.w);
% Build space-time 1-D vectors
y=reshape(xC,N*M, L);

%% Apply weights
zRef = single( abs(conj(wRef)*y));
zRefNorm    =  single(abs(zRef)./max(abs(zRef))); % Normalized

%% Plots
close

fprintf('\n -- <strong>NOTE</strong>: Ploting STAP results of Cuboid #%d\n', sNbr);
f =figure('Name','STAP Simulation Plots','NumberTitle','off','WindowStyle', 'docked');
%f.Position = paramSTAP(sNbr).figurePos; 

% Z Vector Plots
subplot(2, 2, 1);
xAxis  = 1:rangeGateWidth:maxRange;

plot(xAxis, zRefNorm, '--b');
grid on
hold on

%% -- AIEngine-calculated Z


maxIdx  = find(zNorm==max(zNorm));
plot(xAxis, zNorm, ':r',  xAxis(maxIdx), zNorm(maxIdx), 'rv', 'MarkerSize',10, 'LineWidth',1.2);

%% Legend and axis
peakRange = (maxIdx-1) * rangeGateWidth;
strg1 = strcat ("Actual Target Range = ", num2str(round(targetRange)),"m");
strg2 = strcat ("Resolution = +/-", num2str(rangeGateWidth),"m");
text(peakRange+100*rangeGateWidth, 0.6, strg1,'Color','black','FontSize',8 )
text(peakRange+100*rangeGateWidth, 0.53, strg2,'Color','black','FontSize',8 )
trangelgd = strcat('Peak @ ', num2str(peakRange),"m");
legend(  'MATLAB', 'AI-Engine',trangelgd)
xlabel('Range (m)');
ylabel('Normalized Magnitude');
title ('ADPCA Canceller Output')

% 

%% -- Matlab Vs AiEngine
subplot(2, 2, 3);

% Error Vector: Single to single
eVec  =  ((real(zRefNorm) - real(zNorm)).^2 + (imag(zRefNorm) - imag(zNorm)).^2);

% Error Vector: Rounded Single to Single
eVecRoundedS2S = round(eVec, significantBits);

if strcmp(errPlot, "s2S")
    vecToPlot =   eVec;
    label="Error Vector- SinglePrecision";
    
elseif strcmp(errPlot, "roundedS2S")
    vecToPlot =   eVecRoundedS2S;
    label= strcat( "Error Vector -- ", num2str(significantBits), " Significant Decimals");
end

p=12;
e = zeros(p,length(zRefNorm));
for i=1:p
e(i,:)=ismembertol(zRefNorm, zNorm,10^-(i));
end
[r, ~] = (find(e==0));
m = min(r);

h=pcolor(e);
%colormap(gray(2));
set(h, 'EdgeColor', 'none');
axis ij;
% %axis square
% % plot(xAxis, vecToPlot, '-m');
ylabel('Decimal Digit');
xlabel('Range Index (Range/Resolution)');
title ('ADPCA Canceller Output: Matlab vs. AI-Engine');
text(1500,2, 'Digits Match','Color','green','FontSize',8 )
text(1500,10, 'Digits MisMatch', 'Color','red','FontSize',8 );
 
line([0 L],[m m],'color','r');
strg = ['AI-Engine and MATLAB Match up to' ' ' num2str(m) ' Decimal Digits'];
text( 300, m-0.5, strg, 'color','w');

% legend('Decimal Digit Match')

%%  
colormap (jet);
subplot(2, 2, [2, 4]);
plotAngDopResp(reshape(wRef.', N,M), paramSTAP(sNbr));


end

