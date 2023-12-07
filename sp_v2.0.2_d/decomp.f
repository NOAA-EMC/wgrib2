c     from netlib: http://www.netlib.org
      subroutine decomp(ndim,n,a,cond,ipvt,work)
c
      integer ndim,n
      double precision a(ndim,n),cond,work(n)
      integer ipvt(n)
c
c     decomposes a double precision matrix by gaussian elimination
c     and estimates the condition of the matrix.
c
c     use solve to compute solutions to linear systems.
c
c     input..
c
c        ndim = declared row dimension of the array containing  a.
c
c        n = order of the matrix.
c
c        a = matrix to be triangularized.
c
c     output..
c
c        a  contains an upper triangular matrix  u  and a permuted
c          version of a lower triangular matrix  i-l  so that
c          (permutation matrix)*a = l*u .
c
c        cond = an estimate of the condition of  a .
c           for the linear system  a*x = b, changes in  a  and  b
c           may cause changes  cond  times as large in  x .
c           if  cond+1.0 .eq. cond , a is singular to working
c           precision.  cond  is set to  1.0d+32  if exact
c           singularity is detected.
c
c        ipvt = the pivot vector.
c           ipvt(k) = the index of the k-th pivot row
c           ipvt(n) = (-1)**(number of interchanges)
c
c     work space..  the vector  work  must be declared and included
c                   in the call.  its input contents are ignored.
c                   its output contents are usually unimportant.
c
c     the determinant of a can be obtained on output by
c        det(a) = ipvt(n) * a(1,1) * a(2,2) * ... * a(n,n).
c
      double precision ek, t, anorm, ynorm, znorm
      integer nm1, i, j, k, kp1, kb, km1, m
      double precision dabs, dsign
c
      ipvt(n) = 1
      if (n .eq. 1) go to 80
      nm1 = n - 1
c
c     compute 1-norm of a
c
      anorm = 0.0d0
      do 10 j = 1, n
         t = 0.0d0
         do 5 i = 1, n
            t = t + dabs(a(i,j))
    5    continue
         if (t .gt. anorm) anorm = t
   10 continue
c
c     gaussian elimination with partial pivoting
c
      do 35 k = 1,nm1
         kp1= k+1
c
c        find pivot
c
         m = k
         do 15 i = kp1,n
            if (dabs(a(i,k)) .gt. dabs(a(m,k))) m = i
   15    continue
         ipvt(k) = m
         if (m .ne. k) ipvt(n) = -ipvt(n)
         t = a(m,k)
         a(m,k) = a(k,k)
         a(k,k) = t
c
c        skip step if pivot is zero
c
         if (t .eq. 0.0d0) go to 35
c
c        compute multipliers
c
         do 20 i = kp1,n
             a(i,k) = -a(i,k)/t
   20    continue
c
c        interchange and eliminate by columns
c
         do 30 j = kp1,n
             t = a(m,j)
             a(m,j) = a(k,j)
             a(k,j) = t
             if (t .eq. 0.0d0) go to 30
             do 25 i = kp1,n
                a(i,j) = a(i,j) + a(i,k)*t
   25        continue
   30    continue
   35 continue
c
c     cond = (1-norm of a)*(an estimate of 1-norm of a-inverse)
c     estimate obtained by one step of inverse iteration for the
c     small singular vector.  this involves solving two systems
c     of equations, (a-transpose)*y = e  and  a*z = y  where  e
c     is a vector of +1 or -1 chosen to cause growth in y.
c     estimate = (1-norm of z)/(1-norm of y)
c
c     solve (a-transpose)*y = e
c
      do 50 k = 1, n
         t = 0.0d0
         if (k .eq. 1) go to 45
         km1 = k-1
         do 40 i = 1, km1
            t = t + a(i,k)*work(i)
   40    continue
   45    ek = 1.0d0
         if (t .lt. 0.0d0) ek = -1.0d0
         if (a(k,k) .eq. 0.0d0) go to 90
         work(k) = -(ek + t)/a(k,k)
   50 continue
      do 60 kb = 1, nm1
         k = n - kb
         t = 0.0d0
         kp1 = k+1
         do 55 i = kp1, n
            t = t + a(i,k)*work(i)
   55    continue
         work(k) = t + work(k)
         m = ipvt(k)
         if (m .eq. k) go to 60
         t = work(m)
         work(m) = work(k)
         work(k) = t
   60 continue
c
      ynorm = 0.0d0
      do 65 i = 1, n
         ynorm = ynorm + dabs(work(i))
   65 continue
c
c     solve a*z = y
c
      call solve(ndim, n, a, work, ipvt)
c
      znorm = 0.0d0
      do 70 i = 1, n
         znorm = znorm + dabs(work(i))
   70 continue
c
c     estimate condition
c
      cond = anorm*znorm/ynorm
      if (cond .lt. 1.0d0) cond = 1.0d0
      return
c
c     1-by-1
c
   80 cond = 1.0d0
      if (a(1,1) .ne. 0.0d0) return
c
c     exact singularity
c
   90 cond = 1.0d+32
      return
      end
