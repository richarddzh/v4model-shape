%{
a={'butterfly-18.gif','heart-1.gif','classic-10.gif','camel-12.gif','flatfish-9.gif'};
[rf,out]=MakeSimpleRF(9,0:5:175,[6,6]);
for i = 1:length(a)
  subplot('Position',[0,0.98/length(a)*(i-1)+0.02,0.25,0.98/length(a)-0.01]);
  img = imread(['../mpeg7shape/',a{i}]);
  img = imresize(img, [60,72]);
  img = padarray(img,[10,10],0);
  imshow(img>0);
  subplot('Position',[0.26,0.98/length(a)*(i-1)+0.02,0.25,0.98/length(a)-0.01]);
  [mout,idx,ridge,lmap,lines]=SimpleCell(img,rf);
  imshow(lmap==1);
  subplot('Position',[0.59,0.98/length(a)*(i-1)+0.02,0.4,0.98/length(a)-0.01]);
  plot(lines{1}(:,3));
  set(gca, 'XLim', [1,size(lines{1},1)], 'YLim', [0,180], 'XTick', []);
end
%}
[rf,out]=MakeSimpleRF(9,0:5:175,[6,6]);
a={'horse-1.gif','dog-12.gif';
  'brick-01.gif','brick-02.gif';
  'camel-10.gif','camel-11.gif';
  'bell-20.gif','bell-17.gif';
  'horseshoe-1.gif','horseshoe-6.gif',
  'pocket-1.gif','pocket-5.gif'};
step=[30,30,30,30,30,30];
for i = 1:size(a,1)
  %copyfile(['../mpeg7shape/',a{i,1}],['demopic/',a{i,1}]);
  %copyfile(['../mpeg7shape/',a{i,2}],['demopic/',a{i,2}]);
  img1=imread(['demopic/',a{i,1}]);
  img2=imread(['demopic/',a{i,2}]);
  subplot('Position',[0,1/size(a,1)*(i-1),0.19,1/size(a,1)-0.01]);
  imshow(imresize(img1>0,[60,60]));
  subplot('Position',[0.2,1/size(a,1)*(i-1),0.19,1/size(a,1)-0.01]);
  imshow(imresize(img2>0,[60,60]));
  img1=padarray(img1,[20,20],0);
  img2=padarray(img2,[20,20],0);
  [~,~,~,~,~,line1]=SimpleCell(img1,rf);
  [~,~,~,~,~,line2]=SimpleCell(img2,rf);
  degdif=DegreeDiff(line1(:,3),line2(:,3));
  [p,m]=FreeSpaceMin(degdif,line1,line2);
  subplot('Position',[0.4,1/size(a,1)*(i-1),0.19,1/size(a,1)-0.01]);
  fsmap=degdif/90;
  pmap=imdilate(p==1,true(step(i)));
  fsmap(pmap)=1;
  fsmap=imresize(cat(3,fsmap,degdif/90,degdif/90),[60,60]);
  imshow(fsmap);
  subplot('Position',[0.6,1/size(a,1)*(i-1),0.4,1/size(a,1)-0.01]);
  idx=step(i):(step(i)*2):size(m,1);
  idx2=step(i):(step(i)/2):size(m,1);
  m(:,4)=m(:,4)+max(m(:,1));
  plot(m(idx2,1),-m(idx2,2),'or',m(idx2,4),-m(idx2,5),'og');
  set(gca,'XTick',[],'YTick',[]);
  hold on
  for j=idx
    line(m(j,[1,4]),-m(j,[2,5]));
  end
  hold off
end
