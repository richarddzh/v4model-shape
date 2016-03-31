function result = dompeg7(files)
  result = zeros(length(files),length(files),2);
  for i = 1:length(files)
    for j = 1:length(files)
      fprintf('%d,%d\n', i, j);
      degDiff = DegreeDiff(files(i).line(:,3), files(j).line(:,3));
      [fsp,m] = FreeSpaceMin(degDiff, files(i).line, files(j).line);
      degDiff(fsp~=1) = 0;
      degDiff = max(degDiff, [], 2);
      result(i,j,1) = mean(degDiff);
      a=[m(:,1:2),ones(size(m,1),1)];
      y=m(:,4:5);
      b=(a'*a)^(-1)*a'*y;
      result(i,j,2) = mean(sqrt(sum((a*b-y).^2,2)));
    end
  end
end