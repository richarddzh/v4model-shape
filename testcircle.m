%{
[x,y] = ginput(15);
a = [ones(length(x),1),x,y];
y1 = x.^2 + y.^2;
b = (a'*a)^(-1)*a'*y1;
x0 = b(2)/2;
y0 = b(3)/2;
r = sqrt(b(1)+x0*x0+y0*y0);
rectangle('Position',[x0-r,y0-r,2*r,2*r],'Curvature',[1,1]);
hold on
plot(x,y,'+');
hold off
%}
x=[1,2,3,4];
y=x;
a1 = length(x)*sum(y.*(x-y))-sum(y)*sum(x-y);
b1 = sum(x)*sum(x-y)-length(x)*sum(x.*(x-y));
a = sqrt(a1^2/(a1^2+b1^2))*sign(a1);
b = sqrt(b1^2/(a1^2+b1^2))*sign(b1);
mod(atan2(a,-b)/pi*180,180)
c = -mean(a*x+b*y);
line([0,-c/a],[-c/b,0]);
hold on
plot(x,y,'+');
hold off