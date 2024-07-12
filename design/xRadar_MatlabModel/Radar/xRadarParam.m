% 
% Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
% 
function     paramRadarStap   = xRadarParam(tDeltaX, tDeltaY,     ...
    rDeltaX, rDeltaY, rDeltaZ, ...
    jDeltaX, jDeltaY, jDeltaZ)

%% Author B. Berkane .. Modified vesion from MathWorks
% This function sets the xRadar model parameters


if nargin==0
    tDeltaX=0; tDeltaY=0;
    rDeltaX=0; rDeltaY=0; rDeltaZ=0;
    jDeltaX=0; jDeltaY=0; jDeltaZ=0;
end

%% Environment
propSpeed   = physconst('LightSpeed');   % EW speed in m/s
fc          = 16e9 ;                     % Carrier frequenyc in GHz 
lambda      = propSpeed/fc;              % Wave Length in m

%% Radar Antenna -- BackBaffled Isotropic: Eq. power when Azimut Angles in
%  [–90, 90], and zero power otherwise .
hant = phased.IsotropicAntennaElement('BackBaffled',true);
numAntenna      = 4;  % Nbr of Antenna Elements 
antSpacing      = lambda/2;  
dol             = antSpacing/lambda;      
radarAntenna    = ...
phased.ULA('Element',hant,'NumElements',numAntenna,'ElementSpacing', antSpacing);

%% Constraints - MaxRange and Range Resolution
maxRange = 4800;  % meters 
rangeRes = 2.4;   % +/- rangeRes/2 meters 


%% Waveform parameters
numPulseInt     = 10;                      % Nbr of Pulses
pulseBw         = propSpeed/(2*rangeRes);  % Pulse bandwidth
pulseWidth      = 1/pulseBw;               % Pulse width
prf             = propSpeed/(2*maxRange);  % Pulse repetition frequency
fs              = 2*pulseBw;               % Sampling Frequency
samplesPerFrame = fs/prf;                  % L: Nbr of slow tick in Cuboid
cpi             = numPulseInt/prf;

%% Grid Parameters
fastTimeGrid    = unigrid(0,1/fs,1/prf,'[)');
rangeGates      = propSpeed*fastTimeGrid/2;
rangeGateWidth  = propSpeed/(2*fs);
metersPerSample = rangeGates(2);

%% Transmitter parameters
pd          = 0.9;        % Probability of detection
pfa         = 1e-6;       % Probability of false alarm
tgtRcs      = 1;          % Required target radar cross section
txGain      = 20;
snrMin      = albersheim(pd, pfa, numPulseInt); % Albersheim’s equation  
peakPower   = ((4*pi)^3*noisepow(1/pulseWidth)*maxRange^4*...
db2pow(snrMin))/(db2pow(2*txGain)*tgtRcs*lambda^2);

%% Receiver parameters
rxGain  = 20;    % Gain in dB
refTemp = 290;   % reference temperature in Kelvins

%% Time varying gain parameters
rangeLoss           = 2*fspl(rangeGates,lambda);
referenceLoss       = 2*fspl(maxRange,lambda);

%% Radar platform
vr          = lambda/2*prf;     % in [m/s]
arrayAxis   = [0; 1; 0];        % [x, y, z]
radarVel    = vr/2*arrayAxis;   % m/s:    [Vx, Vy, Vz]
radarPos    = [0;0;1000];       % meters: [x, y, z]

%% Jammer platform
jammerPos   = [1000+jDeltaX-rDeltaX; 1732+jDeltaY-rDeltaY; 1000+jDeltaZ-rDeltaZ];   % meters: [x, y, z]
jammerVel   = [0; 0; 0];            % m/s:    [Vx, Vy, Vz]
effPower    = 100;                  % in Watts    

%% Clutter parameters
clutterTxERP = peakPower*db2pow(txGain);
clutterVel   = norm(radarVel);
clutterGamma = -15;                     %   in dB

%% Target parameters
target1Rcs      = 1;
target1Pos      = [900+tDeltaX-rDeltaX;900+tDeltaY-rDeltaY; 0]; % meters:   [x, y, z] 
target1Vel      = [30; 30; 0];    % m/s:      {Vx; Vy; Vz]
target1Location = global2localcoord(target1Pos,'rs',radarPos);
target1AzAngle  = target1Location(1);
target1ElAngle  = target1Location(2);
target1Range    = target1Location(3);
sp              = radialspeed(target1Pos, target1Vel, radarPos, radarVel);
target1Dop      = 2*speed2dop(sp,propSpeed/fc);

%% STAP algorithm Parameters
guardCells     = 4;
trainingCells  = 2000; 
MainLobDirOpt  = 0;  % MainLobDirOpt=1 => MainLOb := Boresight;
% MainLobDirOpt=2 => MainLOb := Target;
if (MainLobDirOpt == 0)
    % main lobe Boresight
    RxMainLobAz = 0;
    RxMainLobEl = 0;
else
    % main lobe towards target
    RxMainLobAz  = target1AzAngle;
    RxMainLobEl  = target1ElAngle;
end
normSpaFreq         = dol* sin(RxMainLobEl*pi/180)* ...
cos(RxMainLobAz*pi/180);
targetNormDopFreq   = target1Dop/prf;
cUT                 = ceil(target1Range/ ...
rangeGateWidth)+1;

%% Simulation  Parameters
simStep     = 2000;      % nmbr of CPIs
nbrSimSteps = 10;        % nmbr of Steps

%% Environment
paramRadarStap.propSpeed = propSpeed;
paramRadarStap.fc        = fc;
paramRadarStap.lambda    = lambda ;

%% Radar Antenna -- BackBaffled Isotropic: Eq. power when Azimut Angles in
paramRadarStap.radarAntenna = radarAntenna;  % ULA antenna
paramRadarStap.dol          = dol;

%% Constraints - MaxRange and Range Resolution
paramRadarStap.maxRange = maxRange;
paramRadarStap.rangeRes = rangeRes;

%% Waveform parameters
paramRadarStap.pulseBw          = pulseBw;
paramRadarStap.prf              = prf;
paramRadarStap.fs               = fs;
paramRadarStap.samplesPerFrame  = samplesPerFrame;
paramRadarStap.numPulseInt      = numPulseInt;
paramRadarStap.cpi              = cpi;

%% Grid Parameters
paramRadarStap.rangeGateWidth  = rangeGateWidth;
paramRadarStap.metersPerSample = metersPerSample;

%% Transmitter parameters
paramRadarStap.txGain    =  txGain;
paramRadarStap.snrMin    =  snrMin;
paramRadarStap.peakPower =  peakPower;

%% Receiver parameters
paramRadarStap.rxGain  = rxGain;
paramRadarStap.refTemp = refTemp;

%% Time varying gain parameters
paramRadarStap.rangeLoss       = rangeLoss;
paramRadarStap.referenceLoss   = referenceLoss;

%% Radar platform
paramRadarStap.radarVel = radarVel;
paramRadarStap.radarPos = radarPos;

%% Jammer platform
paramRadarStap.jammerPos = jammerPos;
paramRadarStap.jammerVel = jammerVel;
paramRadarStap.effPower  = effPower;

%% Clutter parameters
paramRadarStap.clutterTxERP = clutterTxERP;
paramRadarStap.clutterVel   = clutterVel;
paramRadarStap.clutterGamma = clutterGamma;

%% Target parameters
paramRadarStap.target1Rcs       = target1Rcs;
paramRadarStap.target1Pos       = target1Pos;
paramRadarStap.target1Vel       = target1Vel;
paramRadarStap.RxMainLobAz      = RxMainLobAz;
paramRadarStap.RxMainLobEl      = RxMainLobEl;

paramRadarStap.target1AzAngle   = target1AzAngle;
paramRadarStap.target1ElAngle   = target1ElAngle;
paramRadarStap.target1Range     = target1Range;
paramRadarStap.sp               = sp;
paramRadarStap.target1Dop       = target1Dop;

%% STAP algorithm Parameters
paramRadarStap.guardCells           = guardCells;
paramRadarStap.trainingCells        = trainingCells;
paramRadarStap.targetNormDopFreq    = targetNormDopFreq;
paramRadarStap.normSpaFreq          = normSpaFreq;
paramRadarStap.cUT                  = cUT;

%% Simulation  Parameters
paramRadarStap.simStep      = simStep;
paramRadarStap.nbrSimSteps  = nbrSimSteps;

%% Assign variables to the paramStap variable in base workspace
assignin('base','paramSTAP',paramRadarStap);

end
