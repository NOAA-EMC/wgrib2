c     from netlib: http://www.netlib.org
      subroutine solve(ndim, n, a, b, ipvt)
c
      integer ndim, n, ipvt(n)
      double precision a(ndim,n),b(n)
c
c   solution of linear system, a*x = b .
c   do not use if decomp has detected singularity.
c
c   input..
c
c     ndim = declared row dimension of array containing a .
c
c     n = order of matrix.
c
c     a = triangularized matrix obtained from decomp .
c
c     b = right hand side vector.
c
c     ipvt = pivot vector obtained from decomp .
c
c   output..
c
c     b = solution vector, x .
c
      integer kb, km1, nm1, kp1, i, k, m
      double precision t
c
c     forward elimination
c
      if (n .eq. 1) go to 50
      nm1 = n-1
      do 20 k = 1, nm1
         kp1 = k+1
         m = ipvt(k)
         t = b(m)
         b(m) = b(k)
         b(k) = t
         do 10 i = kp1, n
             b(i) = b(i) + a(i,k)*t
   10    continue
   20 continue
c
c     back substitution
c
      do 40 kb = 1,nm1
         km1 = n-kb
         k = km1+1
         b(k) = b(k)/a(k,k)
         t = -b(k)
         do 30 i = 1, km1
             b(i) = b(i) + a(i,k)*t
   30    continue
   40 continue
   50 b(1) = b(1)/a(1,1)
      return
      end
