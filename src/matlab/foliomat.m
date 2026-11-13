%*******************************************************
%  Mosel Matlab Example Problems
%  =============================
%
%  file foliomat.m
%  `````````````````````
%  Modeling a small LP problem 
%  to perform portfolio optimization.
%  
% (c) 2014 Fair Isaac Corporation
%     author: L.Bertacco, Apr. 2014
%*******************************************************

RET  = [5 17 26 12 8 9 7 6 31 21];
RISK = [2 3 4 9 10];
NA   = [1 2 3 4];
[r,e]=moselexec('foliomat.mos');
objval
frac'