%*******************************************************
%  Mosel Matlab Example Problems
%  =============================
%
%  file fibonacci.m
%  ``````````````````
%  Fibonacci function for the fib_relay.mos program
%  
% (c) 2014 Fair Isaac Corporation
%     author: L.Bertacco, Apr. 2014
%*******************************************************

function f=fibonacci(n)   
  if n<2, f=n; return, end
  s=[0 1];
  for i=2:n, s=[s(2) sum(s)]; end  
  f=s(2);
end