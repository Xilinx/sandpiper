% 
% Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
% 

function [celCovMtx] = cellCovMtx(stCube, cell)
 
%% Author Bachir B./Fidus

% Initialize the Covariance Matrix to all zeros
% Effective Cell Covariance matrix is a 2N*(M-1) square matrix
[N, M, ~] = size(stCube);
celCovMtx = zeros(2*N*(M-1), (M-1)*2*N);

% Extract the stMtx for input cell
stMtx= stCube(:,:,cell);
% Construct M-1 2-pulse covariance matrices
for k=1:M-1
    %k,k+1 2-pulse covariance matrix
    twoPulseCov  = conj([stMtx(:,k);stMtx(:,k+1)]) * [stMtx(:,k);stMtx(:,k+1)].';
    celCovMtx(1+2*N*(k-1):2*k*N,1+2*N*(k-1):2*k*N) = twoPulseCov;
end

 
