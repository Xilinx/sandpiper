% 
% Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
% 

function adpcaMetrics = adpcaCanceller (xC, adpcaCFG)


%% Author B. Berkane
% Developped for xRadar Phase 2

% This function calls the following functions:
%  - ./ADPCA/trainingSets (Fidus)
%  - ./ADPCA/cpiCovMtx    (Fidus)
%  - ./ADPCA/steeringVector (Fidus)


% Cfg paramters
N              = adpcaCFG.radarAntenna.NumElements;
L              = adpcaCFG.samplesPerFrame;
M              = adpcaCFG.numPulseInt;
gC             = adpcaCFG.guardCells;
tC             = adpcaCFG.trainingCells;
cUT            = adpcaCFG.cUT;
normSpaFreq    = single(adpcaCFG.normSpaFreq);
normDopFreq    = single(adpcaCFG.targetNormDopFreq);


%% Reset Inverted Covariance Matrices 
InvCovMtx  = zeros(2*N*(M-1), (M-1)*2*N);

%% Get the parameters of the training sets --- See traningSets function for details
[trSets, ~] = trainingSets(L, cUT, gC, tC);

%% extract the traning Cuboid
n       = trSets.nSets;
nCells  = 0;
tCuboid = [];tCuboidF = [];
for s=1:n
    sParam  = trSets.setParam(:,s);  % Training Set Parameters
    m       = sParam(1);             % Number of traning cells in s
    sIdx    = sParam(2);             % Start Idx in the training Set
    eIdx    = sParam(3);             % End Idx in the training Set
    nCells  = m + nCells;
    tCuboid  = xC (:,:,sIdx:eIdx);
    tCuboidF  = [tCuboidF; reshape(tCuboid, N*M*m, [])];
end

tCuboid = reshape(tCuboidF, N, M, nCells);

%% Construct effective covariance matrix over the traning sets
covMtx    = single(cpiCovMtx(xC, trSets));
scale_val = 1; %single(tC);

% Invert each of 2Nx2N 2-pulse covariance matrices in CovMtx over CPI
% Construct Gamma matrix
for k=1:M-1
    % Invert k,k-1 matrix
    twoPulseCov  = covMtx(1+2*N*(k-1):2*k*N,1+2*N*(k-1):2*k*N);
    [Q, R] = qrhhr(twoPulseCov);
    twoPulseCovInv = upperTrMtxInv(R) * (conj(Q).');
    invCovMtx(1+2*N*(k-1):2*k*N,1+2*N*(k-1):2*k*N)  = twoPulseCovInv .*scale_val;
    gammaMtx(1+2*N*(k-1):2*k*N,1+N*(k-1):(k+1)*N) = single(eye(2*N));
end

% Construct the Effective Covariance inverted (ECI) matrix
E = (invCovMtx*gammaMtx);
ECI =  gammaMtx.'* E;

% Get Steering Vector
[st, s, t] = steeringVector(N, M, normDopFreq, normSpaFreq);

% Calculate Adpca weights
w = st*ECI;

% output metrics:tCuboid, covMtx, InvCovMtx, gammaMtx, E, ECI, w, st, s, t
adpcaMetrics.tCuboid   = tCuboid;
adpcaMetrics.covMtx    = covMtx;
adpcaMetrics.invCovMtx = invCovMtx;
adpcaMetrics.gammaMtx  = gammaMtx;
adpcaMetrics.E         = E;
adpcaMetrics.ECI       = ECI;
adpcaMetrics.w         = w;
adpcaMetrics.st        = st;
adpcaMetrics.s         = s;
adpcaMetrics.t         = t;

end

