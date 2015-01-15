function [rf,out] = MakeSimpleRF(rfsize, degrees, grid)
  if ~exist('grid','var')
    grid = [1,length(degrees)];
  end
  [x,y] = meshgrid(1:rfsize);
  x = x - ceil(rfsize/2);
  y = y - ceil(rfsize/2);
  rf = zeros(rfsize,rfsize,length(degrees));
  for i = 1:length(degrees)
    a = degrees(i) * pi / 180;
    x1 = x * cos(a) - y * sin(a);
    y1 = x * sin(a) + y * cos(a);
    rf(:,:,i) = sin(y1*1.5*pi/rfsize) .* exp(-(x1.^2+y1.^2)/rfsize/rfsize*7);
  end
  out = ones(grid(1)*73-1,grid(2)*73-1,3);
  for x = 1:grid(2)
    for y = 1:grid(1)
      k = x + (y-1)*grid(2);
      red = imresize(rf(:,:,k),[72,72]);
      blue = red;
      green = red * 0;
      red(red<0) = 0;
      red = red / max(red(:));
      blue(blue > 0) = 0;
      blue = blue / min(blue(:));
      out((y-1)*73+(1:72),(x-1)*73+(1:72),:) = cat(3,red,green,blue);
    end
  end
end