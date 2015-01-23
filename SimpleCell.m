function [mout,idx,ridge,lmap,lines,line] = SimpleCell(img, rf)
  %tic
  if size(img,3) == 3
    img = rgb2gray(img);
  end
  img = double(img(:,:,1)) / double(max(img(:)));
  out = zeros(size(img,1),size(img,2),size(rf,3));
  for i = 1:size(rf,3)
    out(:,:,i) = imfilter(img, rf(:,:,i));
  end
  out = abs(out);
  sout = sum(out, 3);
  sout(sout < 1) = 1;
  [mout,idx] = max(out, [], 3);
  mout = mout .^ 2 ./ sout;
  ridge = FindRidge(mout, 1);
  ridge = mout>mean(mout(:))*1.5 & ridge;
  [lmap,lines] = FindLine(ridge, (idx-1)*(180/size(rf,3)), 18, 3);
  linelen = zeros(1, length(lines));
  for i = 1:length(lines)
    linelen(i) = size(lines{i},1);
  end
  [ll,lineidx] = max(linelen);
  line = lines{lineidx};
  [temp,startp] = min(line(:,1).^2+line(:,2).^2);
  [temp,dirp] = max(line(:,2)-line(:,1));
  if mod(dirp-startp, ll) > ll/2
    startp = ll - startp + 1;
    line = flipud(line);
  end
  line = [line(startp:ll,:);line(1:startp-1,:)];
  %toc
end