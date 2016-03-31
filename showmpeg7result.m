load result.mat
[~,idx] = sort(result,2);
%{
for i = 1:size(result,1)
  img=zeros(65,64*11+10,3);
  tidx(i,:) = truth(i,idx(i,:));
  for j = 1:11
    img2=imread(['../mpeg7shape/',files(idx(i,j)).name]);
    img2=imresize(img2,[62,62]);
    img2=(img2(:,:,1)==0);
    img2=padarray(img2,[1,1],1);
    if tidx(i,j)
      img(1:64,65*(j-1)+(1:64),1)=img2;
    else
      img(1:64,65*(j-1)+(1:64),1)=1;
    end
    img(1:64,65*(j-1)+(1:64),2)=img2;
    img(1:64,65*(j-1)+(1:64),3)=img2;
  end
  imwrite(img,['mpeg7result/',files(i).name,'.png']);
end
%}
catid = [files.catid];
for i=1:71
  cfiles = files(catid==i);
  if isempty(cfiles)
    continue
  end
  fprintf('%s: %f\n', cfiles(1).label, sum(sum(tidx(catid==i,1:40)))/400);
end