#pragma once

#if !defined(Q_EXPECT)
#  ifndef QT_NO_DEBUG
#    define Q_EXPECT(cond) ((!(cond)) ? qt_assert(#cond,__FILE__,__LINE__) : qt_noop())
#  else
#    define Q_EXPECT(cond) (cond)
#  endif
#endif

#if !defined(Q_EXPECT_X)
#  ifndef QT_NO_DEBUG
#    define Q_EXPECT_X(cond, where, what) ((!(cond)) ? qt_assert_x(where, what,__FILE__,__LINE__) : qt_noop())
#  else
#    define Q_EXPECT_X(cond, where, what) (cond)
#  endif
#endif
