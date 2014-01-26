close all; clear all;

function P2 = skip_short(P)
  L = diff(P);
  L = sqrt(L(:,1).^2 + L(:,2).^2);
  P2 = P;
  P2(find(L < 2) + 1, :) = [];
end

function A = angles(P)
  Q2 = diff(P);
  A = Q2(:,2) ./ sqrt(Q2(:,1).^2 + Q2(:,2).^2);
  A = asin(A);
  A = (Q2(:,1) < 0) .* sign(A) * pi - A;
  T = diff(A);
  T = T - sign(T) .* floor(abs(T) / (2 * pi));
  A = cumsum(T);
  A = A / pi * 180;
end

function res = convpadded(data, kernel, type)
  sz = floor(length(kernel) / 2);
  data0 = repmat(data(1), sz, 1);
  data2 = repmat(data(end), sz, 1);
  res = conv([data0; data; data2], kernel, type);
  res = res(sz + 1:end - sz);
end

D = dlmread('../curve.csv', ',');
M = D(:,1:2);
T = D(:,3);

% length 3 / sigma 6
kernel = [0.3317865678454581, 0.33642686430908364, 0.3317865678454581];

MC = skip_short(M);
MC(:,1) = convpadded(MC(:,1), kernel, 'same');
MC(:,2) = convpadded(MC(:,2), kernel, 'same');

M1 = diff(MC, 1);
M1C = M1;
M1C(:,1) = convpadded(M1(:,1), kernel, 'same');
M1C(:,2) = convpadded(M1(:,2), kernel, 'same');

M2 = diff(M1C, 1);
M2C = M2;
M2C(:,1) = convpadded(M2(:,1), kernel, 'same');
M2C(:,2) = convpadded(M2(:,2), kernel, 'same');

subplot(3,2,1)
plot(M(:,1), M(:,2))
hold on
plot(MC(:,1), MC(:,2), 'r')
title('Curve')

subplot(3,2,3)
plot(M1(:,1), M1(:,2))
hold on
plot(M1C(:,1), M1C(:,2), 'r')
title('Curve''s first derivative')

subplot(3,2,5)
plot(M2(:,1), M2(:,2))
hold on
plot(M2C(:,1), M2C(:,2), 'r')
title('Curve''s second derivative')

subplot(3,2,2)
plot(angles(MC))
subplot(3,2,4)
plot(angles(M1C))
subplot(3,2,6)
plot(angles(M2C))
