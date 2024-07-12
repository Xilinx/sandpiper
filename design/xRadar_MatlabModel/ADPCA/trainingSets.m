% 
% Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
% 

function [trSets, eFlag] = trainingSets(L, tRangeIdx, gC, tC)

%% Author B. Berkane
% This functions finds the training cells in the space-time matrix

% Function inputs: 
%   L         := nbr of samples (fast-time ticks)
%   gC        := Nbr of Guard Cells
%   tC        := Nbr of training cells
%   tRangeIdx := the target range index  >= 1, <= L

% The position of the taget range determines the number of contiguous
% training sets: 2 or 3. This returns:
% 1.    The number of sets,
% 2.    The nbr of elements in each set, and 
% 3.    The start and end idex of each set

% Function outputs: trSets,  eFlag
%   eFlag     := error falg in {true, false}
%   trSets - Struct :
%       trSets.nSets        := nbr of contiguous sets
%       trSets.setParam     := 3x3 matrix: 
%                                 n1 n2 n3
%                                 l1 l2 l3
%                                 r1 rr r3
%        with n<#>      := nbr of elements of Set <#>
%             l<#>/r<#> := left/right index boundaries of Set s<#>

m1             = mod(tC, 2);
m2             = mod(gC, 2);
trSets         = [];
eFlag          = false;  % error flag

%% Nbr of training cells is not even
if (m1 == 1 || m2 == 1)
    fprintf("\n -- Error: Nbr of Traning  and/or Guard Cells Must be an Even Number  \n");
    eFlag  = true;    
end

%% Nbr of training cells exceeds the number of samples (fast-time ticks)  
if (tC+gC > L)
    fprintf("\n -- Error: Nbr of Traning Cells + Guard Cells is greater than the number of samples  \n");
    eFlag  = true;    
end

%% Determine the left and right boundaries of the training sets in a virtual 3*L Cuboid
rightB  = (tRangeIdx+L) + (gC+tC)/2;
leftB   = (tRangeIdx+L) - (gC+tC)/2;

%% WRap to the start of cuboid
if (rightB > 2*L)
    d1 = rightB-2*L;
    if (d1 >= tC/2)
        nSets = 2;
        set1  = [d1-(tC/2)+1, d1];  
        set2  = [tRangeIdx-(gC+tC)/2, tRangeIdx-1-gC/2]; 
        set3  = [-1, -2];
    else
        nSets  = 3;
        set1   = [1, d1];
        set2   = [tRangeIdx-(gC+tC)/2, tRangeIdx-1-gC/2];
        set3   = [tRangeIdx+gC/2+1, L]; 
    end
end

%% Wrap to the end of cuboid
if (leftB < L)
    d1 = (L-leftB+1);
    
    if (d1 >= tC/2)
        nSets = 2;
        set1  = [tRangeIdx+(gC/2)+1,  tRangeIdx+(gC+tC)/2];
        set2  = [leftB, leftB+(tC/2)-1];  
        set3  = [-1, -2];
    else
        nSets = 3;
        set1 = [1, (tC/2)-d1];
        set2 = [tRangeIdx+(gC/2)+1,  tRangeIdx+(gC+tC)/2];
        set3 = [leftB, L];
    end     
end

%% Training within the boudaries of the cuboid
if ((rightB <= 2*L) && (leftB >=L))
    nSets = 2;
    set1  = [tRangeIdx-(gC+tC)/2, tRangeIdx-1-gC/2];
    set2  = [tRangeIdx+(gC/2)+1,  tRangeIdx+(gC+tC)/2];
    set3  =[-1 -2];
end

%% Ouput the results
setParam = [  [set1(2)-set1(1)+1;set1.'], ...
              [set2(2)-set2(1)+1;set2.'], ...
              [set3(2)-set3(1)+1;set3.'] ];
trSets.nSets    = nSets;
trSets.setParam = setParam;

end
