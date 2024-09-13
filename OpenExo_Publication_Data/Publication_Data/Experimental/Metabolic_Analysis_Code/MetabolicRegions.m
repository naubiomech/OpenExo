function [TenMinRegion,StableRegion,MinRegion] = MetabolicRegions(x,V02,VC02,BW)

% Calculate the regions at the 10 minute mark (alternatively the last 6
% points in the metabolic data), the most stable region (the 6 consecutive
% points between the 6 min region "x" and the 10 minute region that have
% the lowest standard deviation), and the minimum region (the 6 consecutive
% points between the 6 min and 10 min region that have the lowest mean
% value) for comparisons.
%
% x is the END of the 6 minute region and must be selected manually using
% the get_meta_data.m script since metabolic data files can be different
% lengths and do not always start at minute zero.
%
% V02 and VC02 are the complete spans of data from the selected file.
%
% BW is the mass of the participant.
%
% Each returned variable is astruct that contains the region of raw V02
% and VC02 data as well as the mean and standard deviation of the metabolic
% cost. This cost must be normalized by speed later in the
% Omnia_PostProcess.m script.

%%% Ten Minute Region
avg_window = 5; %60 seconds
stop = round(x(2)/10);
start = round(x(2)/10)-avg_window;
TenMinV02 = V02(start:stop);
TenMinVC02 = VC02(start:stop);
meanTenMinV02 = mean(TenMinV02);
sdTenMinV02 = std(TenMinV02);
meanTenMinVC02 = mean(TenMinVC02);
sdTenMinVC02 = std(TenMinVC02);
TenMinRaw = {TenMinV02, meanTenMinV02, sdTenMinV02;...
            TenMinVC02, meanTenMinVC02, sdTenMinVC02};
TenMinCostMean = (16.58*meanTenMinV02 + 4.51*meanTenMinVC02)/(60*BW);
TenMinCostSD = (16.58*sdTenMinV02 + 4.51*sdTenMinVC02)/(60*BW);
TenMinCost = {TenMinCostMean;TenMinCostSD};
TenMinRegion.Cost=TenMinCost;
TenMinRegion.RawData=TenMinRaw;

%%% Stable & Minimum Region
minRegionMean = 10000;
stableRegionSD = 10000;
V02_short = V02(round(x(1)/10)-avg_window:stop);
VC02_short = VC02(round(x(1)/10)-avg_window:stop);
for i = 6:length(V02_short)
    V02Region = V02_short(i-avg_window:i);
    VC02Region = VC02_short(i-avg_window:i);
    meanV02 = mean(V02Region);
    sdV02 = std(V02Region);
    meanVC02 = mean(VC02Region);
    sdVC02 = std(VC02Region);
    meanMet = (16.58*meanV02 + 4.51*meanVC02)/(60*BW);
    sdMet = (16.58*sdV02 + 4.51*sdVC02)/(60*BW);
    if meanMet < minRegionMean % Minimum metabolic cost
        min_meanV02 = meanV02;
        min_sdV02 = sdV02;
        min_meanVC02 = meanVC02;
        min_sdVC02 = sdVC02;
        minRegionMean = meanMet;
        minRegionSD = sdMet;
        st_min = i-avg_window;
    end
    if sdMet < stableRegionSD % Minimum metabolic cost standard deviation (stability)
        stable_meanV02 = meanV02;
        stable_sdV02 = sdV02;
        stable_meanVC02 = meanVC02;
        stable_sdVC02 = sdVC02;
        stableRegionSD = sdMet;
        stableRegionMean = meanMet;
        st_stable = i-avg_window;
    end
end
minRegionCost = {minRegionMean;minRegionSD};
minRegionV02 = V02_short(st_min:st_min+avg_window);
minRegionVC02 = VC02_short(st_min:st_min+avg_window);
minRegionRaw = {minRegionV02, min_meanV02, min_sdV02;...
                minRegionVC02, min_meanVC02, min_sdVC02};
MinRegion.Cost=minRegionCost;
MinRegion.RawData=minRegionRaw;
stableRegionCost = {stableRegionMean;stableRegionSD};
stableRegionV02 = V02_short(st_stable:st_stable+avg_window);
stableRegionVC02 = VC02_short(st_stable:st_stable+avg_window);
stableRegionRaw = {stableRegionV02, stable_meanV02, stable_sdV02;...
                stableRegionVC02, stable_meanVC02, stable_sdVC02};
StableRegion.Cost=stableRegionCost;
StableRegion.RawData=stableRegionRaw;
