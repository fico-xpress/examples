%*******************************************************
%  Mosel Matlab Example Problems
%  =============================
%
%  file example_m5.m
%  ``````````````````
%  Initialization from a MATLAB sparse matrix
%  
% (c) 2014 Fair Isaac Corporation
%     author: L.Bertacco, Apr. 2014
%*******************************************************

mos={
'model "example_m5"                  '
' declarations                       '
'  i,j: range                        '
'  sparse: dynamic array(i,j) of real'
' end-declarations                   '
' initializations from "matlab.mws:" '
'  sparse as "sprand(4,4,.5)"        '
' end-initializations                '
' writeln("sparse is: ", sparse)     '
' writeln("row indices: ", i)        '
' writeln("col indices: ", j)        '
'end-model                           '
};
moselexec('matlab.mws:mos');