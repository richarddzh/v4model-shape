function out = DegreeDiff(v1, v2)
  m1 = v1 * ones(1, length(v2));
  m2 = ones(length(v1), 1) * v2';
  out = abs(m1 - m2);
  out(out>90) = 180 - out(out>90);
end