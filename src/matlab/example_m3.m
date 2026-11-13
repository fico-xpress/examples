%*******************************************************
%  Mosel Matlab Example Problems
%  =============================
%
%  file example_m3.m
%  ``````````````````
%  Executing a Mosel program contained
%  in a MATLAB cell array of strings
%  
% (c) 2014 Fair Isaac Corporation
%     author: L.Bertacco, Apr. 2014, rev. Sep. 2017
%*******************************************************

mos={
'model "example_m3"        '
' uses "mmxprs", "mmnl";   '
' declarations             '
'  a:mpvar                 '
' end-declarations         '
' minimize(a*a-5*a+10)     '
' writeln(getobjval)       '
'end-model                 '
};
moselexec('matlab.mws:mos')
