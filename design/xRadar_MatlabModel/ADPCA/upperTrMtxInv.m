% 
% Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
% 

%% R inversion of an upper-triangular matrix solution
%% Author David Quinn
% Developped for xRadar Phase 2


function R_Inv = upperTrMtxInv(R)
% R inversion of a upper -triangular matrix
n = size(R,1);
R_Inv = zeros(n);
for j=1:n
    R_Inv(j,j) = 1/R(j,j);
    for i=1:j-1
        s = R_Inv(i,i:j-1)*R(i:j-1,j);
        R_Inv(i,j) = -s*R_Inv(j,j);
    end
end


end