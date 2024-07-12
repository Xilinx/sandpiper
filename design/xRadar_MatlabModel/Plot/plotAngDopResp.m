% 
% Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
% 

function  plotAngDopResp (st_2d, paramSTAP)
%% Add Paths
addpath ../. ./ADPCA/ ./PLot/  ./Radar/ ./TcpClientListener/ 

%% Author Bachir B./Fidus
 
nFFT            = paramSTAP.nFFT;
dol             = paramSTAP.dol;
elAngDeg        = paramSTAP.RxMainLobEl;

jammerAzAngDeg = (180/pi)* atan ( ...
                                  paramSTAP.jammerPos(2)/ ...
                                  paramSTAP.jammerPos(1)  ...
                                  );                 
targetAzAngDeg  = (180/pi)* atan ( ...
                                  paramSTAP.target1Pos(2)/ ...
                                  paramSTAP.target1Pos(1)  ...
                                  ); 
targetNormDop   = paramSTAP.targetNormDopFreq;

%% Normalized prf, initialize st_resp
prf = 1;
AngDop_resp = zeros(nFFT,nFFT);

%% Phi sweep steps, set N  and M
[N, M]  = size(st_2d);
step    = 180/nFFT;
phi     = (-90:step:90-step).* (pi/180);
n       = (0:N-1).';

% Angle Spectrum
for i = 1:M
 for k=1:length(phi)
    v = exp(1j*2*pi*dol*n*sin(phi(k))*cos(elAngDeg*pi/180)); % Steering vector @ phi(k)
    Ar(k,i) = (st_2d(:,i)'*v);  % beamforming
 end
end

% Doppler spectrum 
AngDop_resp = (fft(Ar,nFFT,2));
AngDop_resp = fftshift(AngDop_resp,2);


%% Angle/Doppler power spectrum density - dB scale
respPsd = abs(squeeze(AngDop_resp));
peakPsd =  max(respPsd);
AngDop_resp_psd  = 20*log10(respPsd);

%% Plot
angle_axis =  phi.*180/pi;
doppler_axis = flip(linspace(-prf/2,(prf/2)-(prf/nFFT)));
imagesc(angle_axis,doppler_axis, AngDop_resp_psd.');
text(targetAzAngDeg, targetNormDop,'+ \leftarrow Target','Color','Black','FontSize',12 )

strg = strcat ("\bf \fontname{Courier} Jammer Azimut w.r.t Radar");
text(jammerAzAngDeg, -0.35, strg, 'HorizontalAlignment', 'center', 'Color','Black','FontSize',8)
strg = strcat ("\bf \fontname{Courier} = ", num2str(jammerAzAngDeg,2),"deg");
text(jammerAzAngDeg, -0.38, strg, 'HorizontalAlignment', 'center', 'Color','Black','FontSize',8)

set(gca,'YDir','normal'); colormap(jet); colorbar;
xlabel('Azimut Angle (Degrees)'); ylabel('Normalized Doppler Frequency')
title('Angle/Doppler Response - dB')
end