function [Q, R] = qrhhr(A)

% Community Code,Developed originally Arshad Afzal(arshad.afzal@gmail.com) 
% Modifed for Fidus by David Quinn 
%

% Copyright (c) 2019, Arshad Afzal
% All rights reserved.

%  Redistribution and use in source and binary forms, with or without
%  modification, are permitted provided that the following conditions are met:%  

%  * Redistributions of source code must retain the above copyright notice, this
%    list of conditions and the following disclaimer.%  

%  * Redistributions in binary form must reproduce the above copyright notice,
%    this list of conditions and the following disclaimer in the documentation
%    and/or other materials provided with the distribution
%  * Neither the name of  nor the names of its
%    contributors may be used to endorse or promote products derived from this
%    software without specific prior written permission.
%  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
%  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
%  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
%  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
%  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
%  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
%  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
%  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
%  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
%  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


[m,n] = size(A);

% Init 
R = zeros(m, n, 'single');
Q = eye(m, m, 'single');
z = zeros(m, 'single');

for j = 1:m-1
    % Reflection of column vector of A
    y = A(j:end,j);
    w = y +sign(A(j,j))*norm(y)*eye(size(y,1),1);
    v = w/norm(w);
    Dummy = 2*(v * (conj(v).'));
    z(j:end,j:end) = Dummy;
    % Generating Househoulder Matrix
    H = eye(m, 'single') - z;
    % Calculating new matrix A using H*A
    A = H*A;
    % Calculating orthogonal matrix, Q using Q=H1*H2*....*Hn
    Q = Q*H;
    z = zeros(m, 'single');
end
    
% Forming the R matrix, R = A
for i = 1:m
    for j = i:n
        R(i,j) = A(i,j);
    end
end
  
    