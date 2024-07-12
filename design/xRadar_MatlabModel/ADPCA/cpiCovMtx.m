% 
% Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
% 

function cpiCovMtx = cpiCovMtx(stCube, trSets)

%% Author Bachir B./Fidus

% This function calls the following functions:
%  - ./ADPCA/cellCovMtx (Fidus)

% Initialize the CPI Covariance Matrix to all zeros
% Effective CPI Covariance matrix is a 2N*(M-1) square matrix
[N, M, ~] = size(stCube);
cpiCovMtx = zeros(2*N*(M-1), (M-1)*2*N);


nCells  = 0;             % Nbr of training cells
n       = trSets.nSets;  % Nbr of contiguous tranings set in stCube


for s=1:n
    sParam  = trSets.setParam(:,s);  % Training Set Parameters
    m       = sParam(1);             % Number of traning cells in s
    sIdx    = sParam(2);             % Start Idx in the training Set
    eIdx    = sParam(3);             % End Idx in the training Set
    nCells  = m + nCells;            % Trailing Number of training cells
    for cell = sIdx : eIdx  
       celCovMtx  = cellCovMtx(stCube, cell); 
       cpiCovMtx  = celCovMtx + cpiCovMtx;
    end

end

% Average over the number of trianing cells
cpiCovMtx = cpiCovMtx./nCells;

