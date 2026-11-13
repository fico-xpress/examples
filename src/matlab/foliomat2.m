%*******************************************************
%  Mosel Matlab Example Problems
%  =============================
%
%  file foliomat2.m
%  ``````````````````
%  Running a small LP problem with different parameters
%  and postprocessing results
%  
% (c) 2014 Fair Isaac Corporation
%     author: L.Bertacco, Apr. 2014
%*******************************************************

NAMES  ={'treasury' 'hardware' 'theater' 'telecom' 'brewery' 'highways' 'cars'    'bank'      'software' 'electronics'};
RET    =[ 5          17         26        12        8         9          7         6           31         21 ];
DEV    =[ 0.1        19         28        22        4         3.5        5         0.5         25         16 ];
COUNTRY={'Canada'   'USA'      'USA'     'USA'     'UK'      'France'   'Germany' 'Luxemburg' 'India'     'Japan'};

RISK_N  ={'hardware' 'theater' 'telecom' 'software' 'electronics'};
NA_N    ={'treasury' 'hardware' 'theater' 'telecom'};

RISK=cellfun(@(n) strmatch(n,NAMES,'exact'), RISK_N); % find indices of high-risk values among shares
NA  =cellfun(@(n) strmatch(n,NAMES,'exact'), NA_N);   % find indices of shares issued in N.-America

for m=1:9
  moselexec('foliomat2.mos',['MAXRISK=' num2str(m/10)]);
  obj(m)=objval;
  optimal(m)=optsol;
  fracm(m,:)=frac;
end

disp('Results');
disp('Estimated returns:');
disp(table([1:9]'/10,obj','VariableNames',{'MaxRisk' 'Return'}))

disp('Average share utilization:');
disp(table(NAMES',mean(fracm)','VariableNames',{'Share' 'AverageUsage'}))

ribbon(fracm)
title('Share utilization')
set(gca,'XTick',[1:size(fracm,2)])
set(gca,'XTickLabel',NAMES)
set(gca,'YDir','reverse')
set(gca,'YTick',[1:9])
set(gca,'YTickLabel',[1:9]/10)
set(gca,'ZLim',[0,max(reshape(fracm,1,[]))])