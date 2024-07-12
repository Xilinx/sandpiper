% 
% Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
% 

function [ip, port] =  getIpAddr()

%% Author B. Berkane
% Set ip address and port manually here
port = 5000;
ip   = '192.168.1.11';

% The following code extracts the IP address if testing the model with
% another matlab instance. Otherwise get IP address from VCK190.

%if ispc
%     [~, result]  = system('ipconfig');   
%     expression   = '(IPv4 Address. . . . . . . . . . . : )(\d+.)(\d+.)(\d+.)(\d+.)';
%     ipExp        = regexp(result,expression,'tokens');
%     [~, m]       = size(ipExp);                                          
%     if (m==1)  
%         l=1;
%     else
%         l=1; % Change n in l=<n> to select the appropraite IP # if Pc has
%              % more than 1 IPv4 address. Run "system('ipconfig')" in matlab
%              % and set n to the desired IPv4 appearance count; that is if
%              % the desired IPv4 appears second, l=2.
%     end        
%     ip = strcat(ipExp{l:l}{2}, ...
%                 ipExp{l:l}{3}, ...
%                 ipExp{l:l}{4}, ...
%                 ipExp{l:l}{5}  );
% else % Linux ... Set manually or complete the code 
%    
%    % [~, result]  = system('ip address | grep inet');
%     ip = '--NotSet--';
%     %%  TBC .. if matlab is running on Linux
% end


end

        

