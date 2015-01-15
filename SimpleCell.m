function [mout,idx,ridge] = SimpleCell(img, rf)
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
  %toc
end