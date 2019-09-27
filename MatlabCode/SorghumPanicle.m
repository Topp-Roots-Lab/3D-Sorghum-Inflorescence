%This is the code for X-ray imaging processing and extracting 3D/2D sorghum
%panicle features and seed profiles.
%For Branch features, see another folder "IdentifyStemAndBranches".

%integrate all images into 3D matrix H
clear H
file=dir('*.png');
for k=1:length(file)
    I=imread(file(k).name);
    H(:,:,k)=imrotate(I(:,:,1),90); %it is OK to directly use H(:,:,k)=I(:,:,1);
end
save H.mat H -v7.3

%cut the stem 1cm below the first branch;
%use the following two line code to find the m is the index of first bottom branch
%P=max(H,[],3);
%imtool(P)
H(:,m+94:end,:)=[]; %based on the resolution "94" can be changed. 94 pixels here = 1cm.

%Crop marker out. Find the corner index includes the marker;
%It is possible to identify the marker automatically by detecting proper sized
%connected component, but it is faster to find the corner index
%manually here.

%Marker=H(m1:m2,m3:m4,m5:m6); where m1,m2,m3,m4,m5,m6 is the corner index. 

%shift gray scale
par0=round((mean(mean(H(:,:,1)))+mean(mean(H(:,:,end)))+mean(mean(H(1,:,:)))+mean(mean(H(end,:,:))))/4);
H=H+(50-par0);
save H.mat H -v7.3
Marker=Marker+(50-par0);
save Marker.mat Marker -v7.3

%adjust intensity based on Marker
load Marker
load('H.mat')
par1=80;
MarkerVol=zeros(100,1);
for s=50:100
    a=find(Marker>=s);
    MarkerVol(s)=length(a);
end
[m1 m2]=min(abs(MarkerVol-240000)); %240000 should be changed base on the size of your marker
Adj=double(Marker)+(par1-m2);
a=find(Adj>=par1);
upper=mean(Adj(a));
H_adj=uint8((double(H)+(par1-m2)-80)/(upper-80)*10+80); 
save H_adj.mat H_adj -v7.3

%3D Panicle Feature
H=H_adj;
par1=80;
[x y z]=findND(H>=par1);
Volume=length(x);
Depth=max(y)-min(y);
[K CHV]=convhulln([x y z]);
ConvexHullVolume=CHV;
[COEFF SCORE latent]=pca([x y z]);
Elongation=sqrt(latent(2)/latent(1));
Flatness=sqrt(latent(3)/latent(2));
M=max(H,[],2); 
M=permute(M,[1 3 2]); %Top projection
[a b]=find(M>=par1);
[COEFF SCORE]=pca([a b]);
Width=max(SCORE(:,1))-min(SCORE(:,1)); %maximum width
ThreeDPanicleFeature=[Volume,Depth,Width,Width/Dept,ConvexHullVolume,Volume/ConvexHullVolume,Elongation,Flatness]

%2D Panicle Feature
%Assume the filename is 'SAP2005'
P=max(H_adj,[],3);
imwrite(P,'SAP2005.png'); %the filename 'SAP2005.png' can be changed
par1=80;
I=imread('SAP2005.png');
II=zeros(size(I));
a=find(I>=par1);
II(a)=1; %II is the binary
CC = bwconncomp(II);
numPixels = cellfun(@numel,CC.PixelIdxList);
[biggest,idx] = max(numPixels);
BW=zeros(size(II));
BW(CC.PixelIdxList{idx}) = 1; %BW is the binary after removing the noise
stats=regionprops(BW,'Area','ConvexArea','Perimeter','MajorAxisLength','MinorAxisLength','boundingbox');
TwoDPanicleFeature=[stats.Area,stats.MajorAxisLength,stats.MinorAxisLength,stats.MinorAxisLength/stats.MajorAxisLength,stats.ConvexArea,stats.Area/stats.ConvexArea,stats.Perimeter,stats.BoundingBox(4)-stats.BoundingBox(3),4*pi*stats.Area/(stats.Perimeter^2)]

%Seed Morphology
%seed binary 
load H_adj
H=H_adj;
clear H_adj
par1=80;
par2=115;%threshold for the seed, could be changed
clear BW
for k=1:size(H,3)
    h=H(:,:,k);
    w=find(h>par2); 
    hh=zeros(size(h));
    hh(w)=1;
    hh=imfill(hh,'hole');
    BW(:,:,k)=hh;
end
save BW.mat BW -v7.3

[L, NUM] = bwlabeln(BW);
clear LL
LL=histc(L(:),[1:NUM]);
[m1 m2]=sort(LL,'descend');
[f,xi] = ksdensity(LL);
[a1 a2]=max(f); %the maximum likely seed size

ept=find(m1<= xi(a2)/8); %ignore component less than 1/8 size

clear Sigma
centroids_sub1=zeros(ept(1),3);
for j=1:ept(1)
  i=m2(j); A=(L==i);
  [x y z]=findND(A==1);
  centroids_sub1(j,:)=mean([x y z]);
  [COEFF SCORE latent]=pca([x y z]); 
  Sigma(j,:)=latent';
end

save Sigma.mat Sigma
save centroids_sub1.mat centroids_sub1
%detect outliers based on the size and shape
outlier=[];
ind=find(m1>=xi(a2)*1.5);
outlier=[outlier;ind];
[f1,xi1] = ksdensity(Sigma(:,1));
[a11 a21]=max(f1);
[f2,xi2] = ksdensity(Sigma(:,2));
[a12 a22]=max(f2);
[f3,xi3] = ksdensity(Sigma(:,3));
[a13 a23]=max(f3);
ind=find(Sigma(:,1)>xi1(a21)*1.5);
outlier=[outlier;ind];
ind=find(Sigma(:,2)>xi2(a22)*1.5);
outlier=[outlier;ind];
ind=find(Sigma(:,3)>xi3(a23)*1.5);
outlier=[outlier;ind];
ind=find(Sigma(:,1)./Sigma(:,2)>1.5*xi1(a21)/xi2(a22));
outlier=[outlier;ind];
ind=find(Sigma(:,2)./Sigma(:,3)>1.5*xi2(a22)/xi3(a23));
outlier=[outlier;ind];
ind=find(Sigma(:,1)./Sigma(:,3)>1.5*xi1(a21)/xi3(a23));
outlier=[outlier;ind];
outlier=unique(outlier);

%erode and count the seeds  
se = strel('sphere',3); 
A=ismember(L,m2(outlier));
BBW = imerode(A,se);
[L_erode, NUM_erode] = bwlabeln(BBW);
LL_erode=histc(L_erode(:),[1:NUM_erode]);
[m1_erode m2_erode]=sort(LL_erode,'descend');

%Detect outliers and two attached seeds
LL_dilate=zeros(NUM_erode,1);
centroids_sub2=zeros(NUM_erode,3);
clear Sigma_erode
for j=1:NUM_erode
  i=m2_erode(j); A=(L_erode==i);
  BBW = imdilate(A,se);
  [x y z]=findND(BBW==1);
  LL_dilate(j)=length(x);
  centroids_sub2(j,:)=mean([x y z]);
  [COEFF SCORE latent]=pca([x y z]);
  Sigma_erode(j,:)=latent';
end
save Sigma_erode.mat Sigma_erode

outlier_erode=[];
ind=find(LL_dilate>=xi(a2)*1.5);
outlier_erode=[outlier_erode;ind];
ind=find(Sigma_erode(:,1)>xi1(a21)*2);
outlier_erode=[outlier_erode;ind];
ind=find(Sigma_erode(:,2)>xi2(a22)*2);
outlier_erode=[outlier_erode;ind];
ind=find(Sigma_erode(:,3)>xi3(a23)*2);
outlier_erode=[outlier_erode;ind];
ind=find(Sigma_erode(:,1)./Sigma_erode(:,2)>1.5*xi1(a21)/xi2(a22));
outlier_erode=[outlier_erode;ind];
ind=find(Sigma_erode(:,2)./Sigma_erode(:,3)>1.5*xi2(a22)/xi3(a23));
outlier_erode=[outlier_erode;ind];
ind=find(Sigma_erode(:,1)./Sigma_erode(:,3)>1.5*xi1(a21)/xi3(a23));
outlier_erode=[outlier_erode;ind];
outlier_erode=unique(outlier_erode);


SN=ept(1)-length(outlier)+NUM_erode-length(outlier_erode);
seedvol=[m1(setdiff([1:ept(1)],outlier));LL_dilate(setdiff([1:NUM_erode],outlier_erode))];
Totalseedvolume=sum(seedvol);

%seeds binary
SeedBW_sub1=ismember(L,m2(setdiff([1:ept(1)],outlier)));
SeedBW_sub2=ismember(L_erode,m2_erode(setdiff([1:NUM_erode],outlier_erode)));
SeedBW_sub2 = imdilate(SeedBW_sub2,se);
SeedBW=max(SeedBW_sub1,SeedBW_sub2);
save SeedBW.mat SeedBW -v7.3
clear SeedBW_sub1
clear SeedBW_sub2

%center of seeds
centroids_sub1(outlier,:)=[];
centroids_sub2=centroids_sub2(setdiff([1:NUM_erode],outlier_erode),:);
centroids=[centroids_sub1;centroids_sub2];
save centroids.mat centroids
save seedvol.mat seedvol

%Seeds distribution profile 
%Select 100 seeds
threshold=min(centroids(:,2)):(max(centroids(:,2))-min(centroids(:,2)))/10:max(centroids(:,2));
Q25=quantile(seedvol,0.25);
clear A
clear BBW
clear BW
SeedShapeBW=zeros(size(L));
a=m2(setdiff([1:ept(1)],outlier));
for j=1:10
    w=find(centroids(:,2)>=threshold(j)&centroids(:,2)<threshold(j+1)&seedvol>=Q25);
     tmp=find(w>length(a));
    w(tmp)=[];
    r=randperm(length(w));r=r(1:10);    
    SeedShapeBW=SeedShapeBW+ismember(L,a(w(r)));
end
SeedShapeBW=logical(SeedShapeBW);
save SeedShapeBW.mat SeedShapeBW -v7.3

%Seeds number histogram along verticle direction
threshold=min(centroids(:,2)):(max(centroids(:,2))-min(centroids(:,2)))/10:max(centroids(:,2));
y_sn=zeros(10,1);
y_bm=zeros(10,1);
for j=1:10
    w=find(centroids(:,2)>=threshold(j)&centroids(:,2)<threshold(j+1));
    y_sn(j)=length(w);
    y_bm(j)=sum(seedvol(w));
end
dlmwrite('SeedDistribution.txt', [y_sn;y_bm ;y_bm./y_sn]', 'delimiter', '\t');


%Seed Shape 
[p,t]=BuildSphere(3);
[azimuth,elevation,R] = cart2sph(p(:,1),p(:,2),p(:,3));
load SeedShapeBW
[L, NUM] = bwlabeln(SeedShapeBW);
Sigma_ss=zeros(NUM,3);
clear Radii
clear Error
clear Seed3D
for j=1:NUM
  A=(L==j);
  [x y z]=findND(A==1);
  [COEFF SCORE latent]=pca([x y z]); 
  Sigma_ss(j,:)=latent';
  B=A(min(x)-10:max(x)+10,min(y)-10:max(y)+10,min(z)-10:max(z)+10);
  [F V]=isosurface(B,0.9999);
  V=V-repmat(mean(V),length(V),1);
  V=V/norm(V,'fro')*sqrt(length(V));
  [az,el,r] = cart2sph(V(:,1),V(:,2),V(:,3));
  DD=pdist2([az el],[azimuth elevation]);
  [M I]=min(DD);
  Seed3D(j,:)=r(I);
  [center, radii, evecs, v, chi2 ] = ellipsoid_fit_new(V);
  Radii(j,:)=radii';
  Error(j)=chi2/length(V);
end

save Sigma_ss.mat Sigma_ss
save Radii.mat Radii
save Error.mat Error
save Seed3D.mat Seed3D
SeedElongation1=mean(sqrt(Sigma_ss(:,2)./Sigma_ss(:,1)))
SeedFlatness1=mean(sqrt(Sigma_ss(:,3)./Sigma_ss(:,2)))
SeedElongation2=mean(Radii(:,2)./Radii(:,1))
SeedFlatness2=mean(Radii(:,3)./Radii(:,2))
SeedEllipsoidError=mean(Error)


%identify stem and extract stem diameter
%Seedless panicle
load SeedBW
load H_adj
par0=double(max(max(H_adj(:,:,1)))+1);%air intensity
se = strel('sphere',4); 
B = imdilate(SeedBW,se);%dilate to remove rendered boundary
HH=H;
for j=1:size(H,3)
    a=find(B(:,:,j)==1);
    h=HH(:,:,j);
    h(a)=par0;
    HH(:,:,j)=h;
end

FS=zeros(size(HH)); %HH is the seedless panicle
se = strel('sphere',2);
k=size(HH,2);
h=HH(:,k,:);
hh=permute(h,[1 3 2]);
bb = imbinarize(hh,par1/255);%80
CC = bwconncomp(bb);
numPixels = cellfun(@numel,CC.PixelIdxList);
[tmp a]=max(numPixels);
bb=zeros(size(bb));
bb(CC.PixelIdxList{a})=1;
bbb=imfill(bb,'holes');
a=find(bbb==1);
b=find(hh(a)<110);
hh(a(b))=110;
bb2 = imdilate(bb,se);
FS(:,k,:)=hh;

for k=size(HH,2)-1:-1:1
    h=HH(:,k,:);
    hh=permute(h,[1 3 2]);
    w=find(bb2==0);
    hh(w)=0;
    bb = imbinarize(hh,par1/255); %80
    CC = bwconncomp(bb);
    numPixels = cellfun(@numel,CC.PixelIdxList);
    [tmp a]=max(numPixels);
    bb=zeros(size(bb));
    bb(CC.PixelIdxList{a})=1;
    bbb=imfill(bb,'holes');
    a=find(bbb==1);
    b=find(hh(a)<110);
    hh(a(b))=110;
    bb2 = imdilate(bb,se);
    FS(:,k,:)=hh;
end
FS=uint8(FS);%FS is stem after filling holes
FS=max(FS,HH);
save FS.mat FS -v7.3
WriteMRC(single(FS), 1, 'SAP2005.mrc')
%Or the following stem could keep filling stem holes
% load FS
% BW=zeros(size(FS));
% BW(a)=1;
% BW=imfill(BW,'holes');
% b=find(BW==1);
% c=setdiff(b,a);
% FS(c)=110;
% save FS.mat FS -v7.3
% WriteMRC(single(FS), 1, 'SAP2005.mrc')

%Stem diameter
load FS
par1=70;
k=size(FS,2);
h=FS(:,k,:);
hh=permute(h,[1 3 2]);
bb = imbinarize(hh,par1/255);
CC = bwconncomp(bb);
numPixels = cellfun(@numel,CC.PixelIdxList);
[tmp a]=max(numPixels);
bb=zeros(size(bb));
bb(CC.PixelIdxList{a})=1;
stats = regionprops(bb,'MajorAxisLength','MinorAxisLength');
(stats.MajorAxisLength+stats.MinorAxisLength)/2