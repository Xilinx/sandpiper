% 
% Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
% 

function [st, spaVec, tempVec] = steeringVector(N, M, normDop, normSpa) 

%% Author Bachir B./Fidus

tempVec = zeros(1,M);
spaVec  = zeros(1,N);

% Spatial Vector
for k=1:N   
  spaVec(k) = exp(-1j*2*(k-1)*pi*normSpa);      
end


% Temporal Vector
for k=1:M   
  tempVec(k) = exp(1j*2*(k-1)*pi*normDop);     
end

% Space-Time Steering Vector
st = kron(tempVec, spaVec)./((N*M));
end

