% 
% Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
% 

%% Author Bachir B./Fidus
% xRadar - Phase 2

% Note that this function excutes the simulink model but does NOT open it.
% If you wish to open the model type the command:  open('./Radar/xRadar.slx'),
% prior of running this script. Most of the model parameters can be
% modified in ./Radar/xRadarParam.m;

% Functions/objects called/created:
%  - getIpAddr    (Fidus)
%  - tcpclient    (Instrumentation-Ctrl toolbox object)
%  - write        (TCP/IP Object function)
%  - ./Radar/readDataFcn  (Fidus)
%  - ./Radar/setGlobal    (Fidus)
%  - ./Radar/xRadarParam  (Fidus)
%  - ./Radar/plotParam    (Fidus)
% ------------------------------------------------------------------------

% Function executes the follwing simulink Model
%  - ./Radar/xRadar.slx   (Original by Mathwork, Modified by Fidus)
%  This function reads the generated cuboids under ./Cuboids and send them 
%  via a tcp/ip Client connection to the remote Server

%% Add Paths
clear ;
addpath ./ADPCA/ ./PLot/  ./Radar/ ./TcpClientListener

%% Set Your IP Address in getIpAddr.m
%  Modify port number if is not available
[ip, port]   = getIpAddr;

%% Get Sim static parameters, and initialized the distance increments
tDeltaX=0; tDeltaY=0;
rDeltaX=0; rDeltaY=0; rDeltaZ=0;
jDeltaX=0; jDeltaY=0; jDeltaZ=0;

paramSTAP   = xRadarParam();

N              = paramSTAP.radarAntenna.NumElements;
L              = paramSTAP.samplesPerFrame;
M              = paramSTAP.numPulseInt;
gC             = paramSTAP.guardCells;
tC             = paramSTAP.trainingCells;

rSpeed         = paramSTAP.radarVel;
tSpeed         = paramSTAP.target1Vel;
jSpeed         = paramSTAP.jammerVel;

cpi            = paramSTAP.cpi;
simStep        = paramSTAP.simStep;
nbrSimSteps    = paramSTAP.nbrSimSteps;

sampF          = 125;  % MHz
Reserved       = 0;    %  Reserved 2-byte value

%% Set TCP parameters
txHeaderNbytes = 32;                % int16
rxHeaderNbytes = txHeaderNbytes*2;  % SingleFloating
tcpRxNbytes    = (L)*4*2 + rxHeaderNbytes;

%%  Create tcp client iff it is not already up and running



if exist('client','var')
    tcpConnExit = true;
else
    tcpConnExit = false;
    try
        client = tcpclient(ip,port);
    catch
    end
end


if exist('client','var')
    %% Open Client connection iff it does not exist
    if (tcpConnExit == true)
        warn = strcat ("\n -- <strong>NOTE</strong>: TCP/IP Client connection to remote IP Address ", ip, " and remote Port #%d is up and running\n");
        fprintf(warn, port);
    else
        warn = strcat ("\n -- <strong>NOTE</strong>: Created TCP/IP Client connection to remote IP Address ", ip, " and remote Port #%d \n");
        fprintf(warn, port);
    end
    

    %% 1st Loop ... read the generated Cuboids
    for k=1:nbrSimSteps
        
        %% run model with increment in position of the radar and the target
        paramSTAP   = xRadarParam( tDeltaX, tDeltaY,          ...
                                   rDeltaX, rDeltaY, rDeltaZ, ...
                                   jDeltaX, jDeltaY, jDeltaZ);
        
        %% Add plotting parameters to paramSTAP record
        paramPlot = plotParam ();
        paramSTAP.figurePos       = paramPlot.figurePos;
        paramSTAP.nFFT            = paramPlot.nFFT;
        paramSTAP.errPlot         = paramPlot.errPlot;
        paramSTAP.significantBits = paramPlot.significantBits;       
        
        %% Determine the increment in covered distance for the radar-platform,
        %% Keep a copy of updated parameters in simParam
        simParam(k) = paramSTAP; % simParam(1= paramSTAP before entering the loop
        
        % Determine the covered distance by radar w.r.t. the initial reference
        rDeltaX = rDeltaX + rSpeed(1)*simStep*cpi;
        rDeltaY = rDeltaY + rSpeed(2)*simStep*cpi;
        rDeltaZ = rDeltaZ + rSpeed(3)*simStep*cpi;
        
        % Determine the covered distance by jammer w.r.t. the initial reference
        jDeltaX = jDeltaX + jSpeed(1)*simStep*cpi;
        jDeltaY = jDeltaY + jSpeed(2)*simStep*cpi;
        jDeltaZ = jDeltaZ + jSpeed(3)*simStep*cpi;
        
        % Determine the covered distance by target w.r.t. the initial reference
        tDeltaX = tDeltaX + tSpeed(1)*simStep*cpi;
        tDeltaY = tDeltaY + tSpeed(2)*simStep*cpi;
        
            
        %% Read the generated cuboid
        tag         = simStep*(k-1);
        fileName    = strcat("./Cuboids/cuboid_", num2str(tag), "x", "cpi.bin");
        
        if isfile(fileName)
            fileNamExist = true;
        end
        if exist('fileNamExist', 'var')
            fileID      = fopen(fileName,'r');
            datIQ(:,k)  = fread(fileID, 'int16');
            fclose(fileID);
            
            % Build the set of cuboids and use as a reference
            iq      =  datIQ(:,k);
            datC    = iq(1:2:end) +  1j*iq(2:2:end);
            spCube  = single(reshape(datC, N,L, M, []));
            stCube  = permute(spCube, [1, 3, 2]);
            stCubes(:,:,:,k) = stCube(:,:,:);   % Set later as Global variable
            clear fileNameExist
        else
            warn = strcat ("\n -- <strong>ERROR</strong>: Cubid File ", fileName, " does not exits, Run genCubes to Generate Cuboids \n");
            fprintf (warn); 
            break
        end
        
    end
    
    %% 2nd Loop ... send Cuboids to TCP/IP server
    for k=1:nbrSimSteps
        
        if exist('fileNamExist', 'var')
            % Time varaying parameters
            cUT            = simParam(k).cUT;
            normSpaFreq    = simParam(k).normSpaFreq;
            normDopFreq    = simParam(k).targetNormDopFreq;
            
            %% Check Training sets Consistancy
            [~, eFlag] = trainingSets(L, cUT, gC, tC);
            
            if (eFlag==1)  % Exit if training parameters are incompatible with cuboid size
                msg = strcat ("\n -- <strong>ERROR</strong>: Training parameters at Simulation Step %d are incompatible with cuboid size, Correct and try again \n"); 
                fprintf(msg, k);
            else
                %% Call ReadDataFCn Function every time TcpRxBytes are received
                % Client is ready and reads every time TcpRxNbytes are received and
                % plots the ADPCA Canceller output
                setGlobalVar(stCubes, simParam, rxHeaderNbytes);
                
                if (paramPlot.enableTcpRxListener==true)
                    configureCallback(client,"byte",tcpRxNbytes, @readDataFcn);
                end
                
                %% TCP/IP payload = header + flatten "cuboid"
                
                % Header is 32 bytes ; each h<i> below is 2-bytes
                h1     = 32767; %  Pkt Identifer
                h2     = k;     %  Cuboid serial number
                
                
                if (nbrSimSteps==1)
                    h3 = 5;  % unique cuboid
                elseif (k==1)
                    h3 = 1;  % 1st cuboid
                elseif (k==nbrSimSteps)
                    h3 = 4;  % last cuboid
                else
                    h3 = 2;  % middle cuboid
                end
                
                h4     = N;
                h5     = M;
                h6     = L;
                
                % Scale and cast to int16 the normalized Doppler and Space
                % freqencies
                normDopFreq_int16 =  int16((2^15)*fi(normDopFreq, 1, 16, 15));
                normSpaFreq_int16 =  int16((2^15)*fi(normSpaFreq, 1, 16, 15));
                
                h7     = normDopFreq_int16;
                h8     = normSpaFreq_int16;
                h9     = gC;              % ADPCA Traning: Number of guard cells
                h10    = tC;              % ADPCA Traning: Number of training cells
                h11    = cUT;             % ADPCA Traning: Cell Under Test
                h12    = sampF;           % Sampling Frequency
                
                h13    = Reserved;
                h14    = Reserved;
                h15    = Reserved;
                h16    = Reserved;
                h = int16([h1 h2 h3 h4 h5 h6 h7 h8 h9 h10 h11, h12, h13, h14, h15, h16]);
                
                %  Build the set of cuboids in format saved in DDR4 and use as a reference
                iq_2 =reshape(datIQ(:,k), 2, []);
                iq_ch1 = iq_2(:,1:4:end);
                iq_ch2 = iq_2(:,2:4:end);
                iq_ch3 = iq_2(:,3:4:end);
                iq_ch4 = iq_2(:,4:4:end);
                
                [c,lm] = size(iq_ch1);
                
                ch1_iq = reshape(iq_ch1, [],c*lm);
                ch2_iq = reshape(iq_ch2, [],c*lm);
                ch3_iq = reshape(iq_ch3, [],c*lm);
                ch4_iq = reshape(iq_ch4, [],c*lm);
                
                
                % Chanelized iq Cuboid
                chan_iq = [ch1_iq, ch2_iq, ch3_iq, ch4_iq];
                
                
                % Form packet to send to TCP/IP server
                to_tcpServer = [h chan_iq];
                
                % Send to TCP/IP server using Instrumentation-Ctrl toolbox object
                fprintf("\n -- <strong>NOTE</strong>: Writing Data of Cuboid #%d to the remote TCP/IP Server \n", k);
                fprintf("\n -- <strong>NOTE</strong>: Cuboid Dimensions: N (# of Antennas)=%d, M (# of Pulses)=%d, L (# of Samples))=%d  \n", N, M, L);
                write(client, to_tcpServer) ;
                
                % save file in binary format
                tag = simStep*(k-1);
                fileName  = strcat("./Cuboids/headerPlusCuboid_", num2str(tag), "x", "cpi.bin");
                fileID    = fopen(fileName,'w');
                fwrite(fileID, to_tcpServer, 'int16');
                fclose(fileID);
                
            end
        else
            break
        end
    end
    
else
    if (strcmp(ip ,'--NotSet--'))
        fprintf ("\n -- <strong>ERROR</strong>: Set manually IP address in getIpAddr.m / Line 11\n");
    else
        msg = strcat ("\n -- <strong>ERROR</strong>: Could not create a TCP/IP Client connection to remote IP Address ", ip, " and remote Port #%d \n");
        fprintf(msg, port);
        fprintf("\n -- <strong>NOTE</strong>: Check IP Address and Port, and make sure the remote server is running\n");
    end
end
