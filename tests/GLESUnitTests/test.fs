#version 300 es
//WebGL

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
precision highp int;
#else
precision mediump float;
precision mediump int;
#endif
#endif

#ifndef REDUCER
#define _GLF_ZERO(X, Y)          (Y)
#define _GLF_ONE(X, Y)           (Y)
#define _GLF_FALSE(X, Y)         (Y)
#define _GLF_TRUE(X, Y)          (Y)
#define _GLF_IDENTITY(X, Y)      (Y)
#define _GLF_DEAD(X)             (X)
#define _GLF_FUZZED(X)           (X)
#define _GLF_WRAPPED_LOOP(X)     X
#define _GLF_WRAPPED_IF_TRUE(X)  X
#define _GLF_WRAPPED_IF_FALSE(X) X
#define _GLF_SWITCH(X)           X
#endif

// END OF GENERATED HEADER
uniform vec2 injectionSwitch;

layout(location=0) out vec4 _GLF_color;

uniform float time;

uniform vec2 resolution;

const float pi = 3.141592653589793238462622433832795;

const float GEAR_PHASE = 0.0958;

const vec3 GEAR_COLOR = vec3(217.0 / 255.0, 128.0 / 255.0, _GLF_IDENTITY(48.0, (48.0) / _GLF_IDENTITY(1.0, max(1.0, 1.0))) / 255.0);

vec2 clog(vec2 v)
{
 do
  {
   if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
    {
     _GLF_color = vec4(38.17, 5.5, _GLF_IDENTITY(1735.0468, 0.0 + (1735.0468)), -3.8);
     if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
      {
       if(_GLF_WRAPPED_IF_FALSE(false))
        {
        }
       else
        {
         if(_GLF_WRAPPED_IF_TRUE(true))
          {
           _GLF_color = vec4(77.54, 1000.4127, -4.7, -34.74);
          }
         else
          {
          }
        }
      }
    }
  }
 while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))));
 if(_GLF_WRAPPED_IF_TRUE(true))
  {
   if(_GLF_DEAD(_GLF_IDENTITY(false, (false) && true)))
    {
     if(_GLF_WRAPPED_IF_TRUE(true))
      {
       if(_GLF_WRAPPED_IF_TRUE(_GLF_IDENTITY(true, (true) || false)))
        {
         if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (_GLF_IDENTITY(gl_FragCoord.y, min(gl_FragCoord.y, _GLF_IDENTITY(gl_FragCoord.y, clamp(gl_FragCoord.y, _GLF_IDENTITY(gl_FragCoord.y, (false ? _GLF_FUZZED(time) : gl_FragCoord.y)), gl_FragCoord.y)))) >= _GLF_IDENTITY(0.0, max(0.0, 0.0))))))
          {
           if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, _GLF_IDENTITY((gl_FragCoord.x < _GLF_ZERO(0.0, injectionSwitch.x)), (true ? (gl_FragCoord.x < _GLF_ZERO(_GLF_IDENTITY(0.0, 0.0 + (0.0)), _GLF_IDENTITY(injectionSwitch.x, clamp(injectionSwitch.x, injectionSwitch.x, _GLF_IDENTITY(injectionSwitch.x, clamp(injectionSwitch.x, injectionSwitch.x, injectionSwitch.x)))))) : _GLF_FUZZED((true != false)))))))
            {
            }
           else
            {
             if(_GLF_WRAPPED_IF_FALSE(false))
              {
              }
             else
              {
               if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.y < _GLF_ZERO(0.0, injectionSwitch.x)))))
                {
                 _GLF_color = min(vec4(20.19, 2324.5046, 3638.2703, -8.9), 6.3);
                }
               if(_GLF_DEAD(false))
                {
                 _GLF_color = trunc(vec4(-69.63, -17.38, 876.162, -797.393));
                }
               _GLF_color = vec4(360.400, 7.6, 62.40, -68.78);
              }
            }
          }
         else
          {
          }
        }
       else
        {
        }
      }
     else
      {
      }
    }
  }
 else
  {
  }
 do
  {
   if(_GLF_WRAPPED_IF_FALSE(_GLF_IDENTITY(false, false || (_GLF_IDENTITY(false, (false) && true)))))
    {
     for(
         int _injected_loop_counter_36 = 1;
         _GLF_WRAPPED_LOOP(_injected_loop_counter_36 != int(_GLF_ZERO(0.0, injectionSwitch.x)));
         _injected_loop_counter_36 --
     )
      {
       if(_GLF_DEAD(false))
        {
         _GLF_color = vec4(-40.15, 42.72, 9.5, 0.7);
        }
      }
    }
   else
    {
     for(
         int _injected_loop_counter_22 = 0;
         _GLF_WRAPPED_LOOP(_injected_loop_counter_22 < _GLF_IDENTITY(1, max(1, 1)));
         _injected_loop_counter_22 ++
     )
      {
       if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (injectionSwitch.x > _GLF_IDENTITY(injectionSwitch.y, clamp(_GLF_IDENTITY(injectionSwitch.y, (injectionSwitch.y) - 0.0), _GLF_IDENTITY(injectionSwitch, (injectionSwitch) - vec2(0.0, 0.0)).y, injectionSwitch.y))))))
        {
        }
       else
        {
         if(_GLF_WRAPPED_IF_TRUE(true))
          {
           do
            {
             for(
                 int _injected_loop_counter_60 = 0;
                 _GLF_WRAPPED_LOOP(_injected_loop_counter_60 != _GLF_IDENTITY(1, min(1, _GLF_IDENTITY(1, max(1, 1)))));
                 _injected_loop_counter_60 ++
             )
              {
               for(
                   int _injected_loop_counter_13 = 1;
                   _GLF_WRAPPED_LOOP(_injected_loop_counter_13 > int(_GLF_ZERO(0.0, injectionSwitch.x)));
                   _injected_loop_counter_13 --
               )
                {
                 if(_GLF_DEAD(false))
                  {
                   for(
                       int _injected_loop_counter_61 = 1;
                       _GLF_WRAPPED_LOOP(_injected_loop_counter_61 > int(_GLF_ZERO(0.0, injectionSwitch.x)));
                       _injected_loop_counter_61 --
                   )
                    {
                     for(
                         int _injected_loop_counter_23 = 0;
                         _GLF_WRAPPED_LOOP(_injected_loop_counter_23 < 1);
                         _injected_loop_counter_23 ++
                     )
                      {
                       if(_GLF_DEAD(false))
                        {
                         do
                          {
                           for(
                               int _injected_loop_counter_37 = 0;
                               _GLF_WRAPPED_LOOP(_injected_loop_counter_37 != 1);
                               _injected_loop_counter_37 ++
                           )
                            {
                             for(
                                 int _injected_loop_counter_32 = 0;
                                 _GLF_WRAPPED_LOOP(_injected_loop_counter_32 < 1);
                                 _injected_loop_counter_32 ++
                             )
                              {
                               if(_GLF_WRAPPED_IF_FALSE(false))
                                {
                                }
                               else
                                {
                                 _GLF_color = (+ exp2(vec4(-812.323, 233.571, 6860.5301, 2.5)));
                                }
                              }
                            }
                          }
                         while(_GLF_WRAPPED_LOOP(_GLF_IDENTITY(_GLF_FALSE(false, _GLF_IDENTITY((gl_FragCoord.y < 0.0), (false ? _GLF_FUZZED(true) : (gl_FragCoord.y < 0.0)))), (true ? _GLF_FALSE(false, (gl_FragCoord.y < 0.0)) : _GLF_FUZZED(true)))));
                        }
                       if(_GLF_DEAD(false))
                        {
                         _GLF_color = vec4(-33.57, 47.59, 155.844, 576.648);
                        }
                      }
                    }
                   do
                    {
                     if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
                      {
                       if(_GLF_WRAPPED_IF_TRUE(_GLF_IDENTITY(true, (_GLF_IDENTITY(_GLF_IDENTITY(true, true && (true)), _GLF_TRUE(true, (_GLF_IDENTITY(gl_FragCoord.y, (gl_FragCoord.y) / 1.0) >= 0.0)) && (true))) && true)))
                        {
                         if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))))
                          {
                          }
                         else
                          {
                           _GLF_color = log(vec4(40.41, 1.1, -0.5, 48.93));
                          }
                        }
                       else
                        {
                        }
                      }
                    }
                   while(_GLF_WRAPPED_LOOP(false));
                   if(_GLF_DEAD(false))
                    {
                     for(
                         int _injected_loop_counter_76 = int(_GLF_ZERO(0.0, injectionSwitch.x));
                         _GLF_WRAPPED_LOOP(_injected_loop_counter_76 < 1);
                         _injected_loop_counter_76 ++
                     )
                      {
                       _GLF_color = vec4(-4464.9891, 8.5, -1570.3203, 4.8);
                      }
                    }
                   for(
                       int _injected_loop_counter_7 = 1;
                       _GLF_WRAPPED_LOOP(_injected_loop_counter_7 != int(_GLF_ZERO(_GLF_IDENTITY(0.0, _GLF_IDENTITY((_GLF_TRUE(true, (gl_FragCoord.y >= 0.0)) ? 0.0 : _GLF_FUZZED(time)), ((_GLF_TRUE(true, (gl_FragCoord.y >= 0.0)) ? 0.0 : _GLF_FUZZED(time))) / 1.0)), _GLF_IDENTITY(injectionSwitch, max(injectionSwitch, injectionSwitch)).x)));
                       _injected_loop_counter_7 --
                   )
                    {
                     do
                      {
                       if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (gl_FragCoord.y >= 0.0))))
                        {
                         for(
                             int _injected_loop_counter_38 = 0;
                             _GLF_WRAPPED_LOOP(_injected_loop_counter_38 < 1);
                             _injected_loop_counter_38 ++
                         )
                          {
                           if(_GLF_WRAPPED_IF_FALSE(false))
                            {
                            }
                           else
                            {
                             _GLF_color = vec4(947.876, 8.7, -614.879, 2816.7989);
                            }
                          }
                        }
                       else
                        {
                        }
                      }
                     while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))));
                     if(_GLF_DEAD(_GLF_IDENTITY(false, (_GLF_FALSE(false, (gl_FragCoord.x < 0.0)) ? _GLF_FUZZED((bvec4(false, true, false, false) == bvec4(false, false, false, false))) : false))))
                      {
                       if(_GLF_WRAPPED_IF_TRUE(true))
                        {
                         for(
                             int _injected_loop_counter_47 = 1;
                             _GLF_WRAPPED_LOOP(_injected_loop_counter_47 != int(_GLF_ZERO(0.0, injectionSwitch.x)));
                             _injected_loop_counter_47 --
                         )
                          {
                           do
                            {
                             for(
                                 int _injected_loop_counter_24 = 1;
                                 _GLF_WRAPPED_LOOP(_injected_loop_counter_24 != int(_GLF_ZERO(0.0, injectionSwitch.x)));
                                 _injected_loop_counter_24 --
                             )
                              {
                               if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (injectionSwitch.x < injectionSwitch.y))))
                                {
                                 _GLF_color = (mat3x4(3.2, 33.35, 692.500, 60.82, 70.98, 62.61, 3502.3774, 17.27, _GLF_IDENTITY(2.7, max(2.7, 2.7)), 6.4, -4722.8676, -8.9) * vec3(-36.21, -8346.7019, 1.3));
                                }
                               else
                                {
                                }
                               if(_GLF_WRAPPED_IF_TRUE(true))
                                {
                                 if(_GLF_DEAD(false))
                                  {
                                   _GLF_color = min(min(vec4(-7.3, -9.9, -4591.9560, 1.1), -8325.4357), -49.23);
                                  }
                                }
                               else
                                {
                                }
                              }
                            }
                           while(_GLF_WRAPPED_LOOP(false));
                          }
                        }
                       else
                        {
                        }
                      }
                    }
                   if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
                    {
                     do
                      {
                       for(
                           int _injected_loop_counter_9 = 0;
                           _GLF_WRAPPED_LOOP(_injected_loop_counter_9 < int(_GLF_ONE(1.0, injectionSwitch.y)));
                           _injected_loop_counter_9 ++
                       )
                        {
                         if(_GLF_DEAD(false))
                          {
                           _GLF_color = exp(vec4(-5163.5020, 912.795, -7.7, 1.3));
                          }
                         if(_GLF_WRAPPED_IF_FALSE(false))
                          {
                          }
                         else
                          {
                           if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (injectionSwitch.x < injectionSwitch.y))))
                            {
                             if(_GLF_WRAPPED_IF_FALSE(false))
                              {
                              }
                             else
                              {
                               if(_GLF_DEAD(false))
                                {
                                 if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (injectionSwitch.x < injectionSwitch.y))))
                                  {
                                   _GLF_color = clamp(vec4(46.43, -46.04, 53.00, _GLF_IDENTITY(-8.6, (true ? -8.6 : _GLF_FUZZED(determinant((+ mat4(6.4, -72.19, 4.5, 7.2, 15.94, -7.0, -137.836, -3253.1376, -538.773, -6.4, 2576.8730, 35.99, -5604.1252, 1.3, -8.1, -8.7))))))), vec4(_GLF_IDENTITY(-6746.0004, (-6746.0004) + 0.0), 6853.7455, -0.8, 65.74), vec4(92.77, -9.1, -9668.5197, -342.123));
                                  }
                                 else
                                  {
                                  }
                                }
                              }
                             for(
                                 int _injected_loop_counter_17 = int(_GLF_ZERO(0.0, injectionSwitch.x));
                                 _GLF_WRAPPED_LOOP(_injected_loop_counter_17 != 1);
                                 _injected_loop_counter_17 ++
                             )
                              {
                               if(_GLF_DEAD(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))))
                                {
                                 if(_GLF_DEAD(false))
                                  {
                                   if(_GLF_WRAPPED_IF_TRUE(true))
                                    {
                                     _GLF_color = vec4(-360.802, -8.8, -68.11, 44.54);
                                    }
                                   else
                                    {
                                    }
                                  }
                                 if(_GLF_WRAPPED_IF_FALSE(_GLF_IDENTITY(false, (_GLF_FALSE(false, (gl_FragCoord.y < 0.0)) ? _GLF_FUZZED(false) : _GLF_IDENTITY(false, false || (false))))))
                                  {
                                  }
                                 else
                                  {
                                   if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))))
                                    {
                                    }
                                   else
                                    {
                                     if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))))
                                      {
                                      }
                                     else
                                      {
                                       do
                                        {
                                         for(
                                             int _injected_loop_counter_39 = 0;
                                             _GLF_WRAPPED_LOOP(_injected_loop_counter_39 != 1);
                                             _injected_loop_counter_39 ++
                                         )
                                          {
                                           if(_GLF_WRAPPED_IF_FALSE(false))
                                            {
                                            }
                                           else
                                            {
                                             _GLF_color = (-6616.3857 * vec4(_GLF_IDENTITY(65.35, (65.35) * 1.0), 600.506, -7.0, -486.575));
                                            }
                                          }
                                        }
                                       while(_GLF_WRAPPED_LOOP(false));
                                      }
                                    }
                                  }
                                 if(_GLF_DEAD(_GLF_FALSE(false, (_GLF_IDENTITY(injectionSwitch, (injectionSwitch) - vec2(0.0, 0.0)).x > injectionSwitch.y))))
                                  {
                                   _GLF_color = vec4(-3.4, 72.09, -1.3, -2.9);
                                  }
                                }
                              }
                             if(_GLF_WRAPPED_IF_FALSE(_GLF_IDENTITY(false, false || (_GLF_IDENTITY(false, true && (false))))))
                              {
                              }
                             else
                              {
                               _GLF_color = vec4(-6.5, -4.6, 3.2, -8.4);
                              }
                            }
                           else
                            {
                            }
                          }
                        }
                       if(_GLF_DEAD(false))
                        {
                         _GLF_color = roundEven(vec3(-6.5, -2.4, -440.286)).rggr;
                        }
                      }
                     while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (_GLF_IDENTITY(gl_FragCoord, min(gl_FragCoord, _GLF_IDENTITY(gl_FragCoord, max(gl_FragCoord, gl_FragCoord)))).y < _GLF_ZERO(0.0, injectionSwitch.x)))));
                    }
                  }
                }
              }
             if(_GLF_WRAPPED_IF_FALSE(false))
              {
              }
             else
              {
               if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (gl_FragCoord.x >= 0.0))))
                {
                 if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))))
                  {
                   _GLF_color = vec4(vec3(-14.90, 6.9, -855.650), -8.0);
                  }
                }
               else
                {
                }
              }
            }
           while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))));
          }
         else
          {
          }
         if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))))
          {
           _GLF_color = vec4(6.8, -5.8, -6.6, 5.0);
          }
        }
      }
    }
  }
 while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))));
 if(_GLF_DEAD(false))
  {
   for(
       int _injected_loop_counter_48 = 1;
       _GLF_WRAPPED_LOOP(_injected_loop_counter_48 > 0);
       _injected_loop_counter_48 --
   )
    {
     _GLF_color = vec4(vec3(349.118, 7.7, _GLF_IDENTITY(6.1, 1.0 * (6.1))), fract(1.7));
    }
  }
 do
  {
   for(
       int _injected_loop_counter_10 = 1;
       _GLF_WRAPPED_LOOP(_injected_loop_counter_10 > 0);
       _injected_loop_counter_10 --
   )
    {
     if(_GLF_DEAD(false))
      {
       do
        {
         _GLF_color = (823.306 - vec4(8094.0426, -2368.5048, -1.1, 355.241));
        }
       while(_GLF_WRAPPED_LOOP(false));
      }
     do
      {
       if(_GLF_DEAD(false))
        {
         if(_GLF_WRAPPED_IF_FALSE(false))
          {
          }
         else
          {
           _GLF_color = sin(vec4(_GLF_IDENTITY(-8.6, (_GLF_TRUE(true, (gl_FragCoord.y >= 0.0)) ? -8.6 : _GLF_FUZZED(GEAR_PHASE))), -0.8, -54.39, 9866.1203));
          }
        }
      }
     while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))));
    }
  }
 while(_GLF_WRAPPED_LOOP(false));
 do
  {
   for(
       int _injected_loop_counter_40 = int(_GLF_ONE(1.0, injectionSwitch.y));
       _GLF_WRAPPED_LOOP(_injected_loop_counter_40 != _GLF_IDENTITY(0, max(0, 0)));
       _injected_loop_counter_40 --
   )
    {
     do
      {
       for(
           int _injected_loop_counter_49 = 0;
           _GLF_WRAPPED_LOOP(_injected_loop_counter_49 != int(_GLF_ONE(1.0, injectionSwitch.y)));
           _injected_loop_counter_49 ++
       )
        {
         for(
             int _injected_loop_counter_18 = int(_GLF_ZERO(_GLF_IDENTITY(0.0, (0.0) - 0.0), injectionSwitch.x));
             _GLF_WRAPPED_LOOP(_injected_loop_counter_18 < _GLF_IDENTITY(int(_GLF_ONE(1.0, injectionSwitch.y)), (true ? _GLF_IDENTITY(int(_GLF_ONE(1.0, injectionSwitch.y)), 0 + (int(_GLF_ONE(1.0, injectionSwitch.y)))) : _GLF_FUZZED(_injected_loop_counter_49))));
             _injected_loop_counter_18 ++
         )
          {
           if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))))
            {
            }
           else
            {
             do
              {
               if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (injectionSwitch.x < injectionSwitch.y))))
                {
                 for(
                     int _injected_loop_counter_0 = int(_GLF_ONE(_GLF_IDENTITY(1.0, 1.0 * (1.0)), injectionSwitch.y));
                     _GLF_WRAPPED_LOOP(_injected_loop_counter_0 != _GLF_IDENTITY(0, max(_GLF_IDENTITY(0, min(0, _GLF_IDENTITY(0, clamp(0, _GLF_IDENTITY(0, max(0, 0)), 0)))), 0)));
                     _injected_loop_counter_0 --
                 )
                  {
                   do
                    {
                     if(_GLF_WRAPPED_IF_TRUE(true))
                      {
                       if(_GLF_WRAPPED_IF_FALSE(_GLF_IDENTITY(false, (_GLF_IDENTITY(false, false || (false))) || false)))
                        {
                        }
                       else
                        {
                         do
                          {
                           if(_GLF_WRAPPED_IF_TRUE(true))
                            {
                             do
                              {
                               return vec2(0.5 * log(_GLF_IDENTITY(v.x, max(_GLF_IDENTITY(v, clamp(v, v, _GLF_IDENTITY(v, clamp(_GLF_IDENTITY(v, (v) / _GLF_IDENTITY(vec2(1.0, 1.0), max(vec2(1.0, 1.0), vec2(1.0, 1.0)))), v, v)))).x, v.x)) * v.x + _GLF_IDENTITY(v.y * v.y, (v.y * v.y) + 0.0)), atan(- v.y, v.x));
                              }
                             while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))));
                            }
                           else
                            {
                            }
                          }
                         while(_GLF_WRAPPED_LOOP(false));
                        }
                      }
                     else
                      {
                      }
                    }
                   while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))));
                  }
                }
               else
                {
                 if(_GLF_WRAPPED_IF_FALSE(false))
                  {
                  }
                 else
                  {
                   if(_GLF_DEAD(_GLF_FALSE(false, _GLF_IDENTITY((gl_FragCoord.y < 0.0), true && ((gl_FragCoord.y < 0.0))))))
                    {
                     for(
                         int _injected_loop_counter_62 = 1;
                         _GLF_WRAPPED_LOOP(_injected_loop_counter_62 != 0);
                         _injected_loop_counter_62 --
                     )
                      {
                       do
                        {
                         if(_GLF_WRAPPED_IF_TRUE(true))
                          {
                           _GLF_color = _GLF_IDENTITY(mod(vec4(4.9, -4299.0121, -529.336, -8772.0843), 10.52), vec4(0.0, 0.0, 0.0, 0.0) + (mod(vec4(4.9, -4299.0121, -529.336, -8772.0843), 10.52)));
                          }
                         else
                          {
                          }
                        }
                       while(_GLF_WRAPPED_LOOP(false));
                      }
                    }
                  }
                }
              }
             while(_GLF_WRAPPED_LOOP(false));
            }
          }
        }
      }
     while(_GLF_WRAPPED_LOOP(false));
    }
  }
 while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))));
}
vec4 gear(vec2 uv, float dir, float phase)
{
 vec2 p = uv - 0.5;
 if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
  {
   do
    {
     if(_GLF_WRAPPED_IF_TRUE(true))
      {
       if(_GLF_DEAD(false))
        {
         for(
             int _injected_loop_counter_41 = 1;
             _GLF_WRAPPED_LOOP(_injected_loop_counter_41 != 0);
             _injected_loop_counter_41 --
         )
          {
           _GLF_color = vec4(-26.02, -15.87, 2.5, -483.152);
          }
        }
      }
     else
      {
      }
    }
   while(_GLF_WRAPPED_LOOP(false));
  }
 else
  {
   if(_GLF_WRAPPED_IF_TRUE(true))
    {
     if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))))
      {
       if(_GLF_WRAPPED_IF_FALSE(false))
        {
        }
       else
        {
         _GLF_color = vec4(-263.828, _GLF_IDENTITY(567.426, clamp(567.426, 567.426, 567.426)), -9.3, 0.7);
        }
       for(
           int _injected_loop_counter_42 = 1;
           _GLF_WRAPPED_LOOP(_injected_loop_counter_42 != 0);
           _injected_loop_counter_42 --
       )
        {
         if(_GLF_DEAD(false))
          {
           _GLF_color = vec4(-754.479, 26.61, -7.0, 1.4);
          }
        }
      }
    }
   else
    {
    }
   if(_GLF_DEAD(_GLF_FALSE(false, (_GLF_IDENTITY(injectionSwitch.x > injectionSwitch.y, (injectionSwitch.x > injectionSwitch.y) && true)))))
    {
     do
      {
       _GLF_color = max(vec4(-2.8, -3610.8705, -965.881, -233.817), 8026.7591);
      }
     while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))));
    }
  }
 float r = length(p);
 float t = fract(time * 0.2);
 do
  {
   if(_GLF_WRAPPED_IF_TRUE(true))
    {
     if(_GLF_WRAPPED_IF_FALSE(false))
      {
      }
     else
      {
       if(_GLF_DEAD(false))
        {
         do
          {
           if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
            {
            }
           else
            {
             if(_GLF_DEAD(false))
              {
               _GLF_color = vec4(-874.987, -9.4, 851.127, -559.587);
              }
            }
          }
         while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))));
         _GLF_color = vec4(-459.577, -2505.1189, 991.908, 4.4);
         do
          {
           if(_GLF_DEAD(false))
            {
             do
              {
               _GLF_color = ((-4.8 / inversesqrt(3022.7963)) + vec4(5.3, 8.4, 0.1, -8.9));
              }
             while(_GLF_WRAPPED_LOOP(_GLF_IDENTITY(false, (false) || false)));
            }
          }
         while(_GLF_WRAPPED_LOOP(false));
        }
       do
        {
         if(_GLF_WRAPPED_IF_TRUE(true))
          {
           do
            {
             do
              {
               if(_GLF_WRAPPED_IF_TRUE(true))
                {
                 if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (gl_FragCoord.x >= 0.0))))
                  {
                   if(_GLF_WRAPPED_IF_FALSE(false))
                    {
                     if(_GLF_DEAD(false))
                      {
                       _GLF_color = (mat2x4(1.2, -9.0, 5.4, 0.7, 44.51, 7.3, -6.6, -85.00) * log(vec2(-2787.8433, 1.2)));
                      }
                    }
                   else
                    {
                     t *= _GLF_IDENTITY(2.0, clamp(2.0, 2.0, _GLF_IDENTITY(2.0, max(2.0, 2.0)))) * pi / 6.0;
                     do
                      {
                       if(_GLF_DEAD(false))
                        {
                         _GLF_color = uintBitsToFloat(uvec4(140394u, 195720u, 128413u, 144083u));
                         if(_GLF_DEAD(false))
                          {
                           _GLF_color = vec2(-0.8, 0.5).xxxx;
                          }
                        }
                      }
                     while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, _GLF_IDENTITY((injectionSwitch.x > injectionSwitch.y), (_GLF_IDENTITY((injectionSwitch.x > injectionSwitch.y), true && ((injectionSwitch.x > injectionSwitch.y)))) && _GLF_TRUE(true, (injectionSwitch.x < injectionSwitch.y))))));
                    }
                  }
                 else
                  {
                  }
                }
               else
                {
                }
              }
             while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.y < _GLF_IDENTITY(_GLF_IDENTITY(0.0, (0.0) - 0.0), clamp(0.0, 0.0, 0.0))))));
            }
           while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))));
          }
         else
          {
          }
        }
       while(_GLF_WRAPPED_LOOP(false));
       for(
           int _injected_loop_counter_25 = 0;
           _GLF_WRAPPED_LOOP(_injected_loop_counter_25 != int(_GLF_ONE(1.0, injectionSwitch.y)));
           _injected_loop_counter_25 ++
       )
        {
         if(_GLF_WRAPPED_IF_FALSE(false))
          {
          }
         else
          {
           if(_GLF_DEAD(_GLF_IDENTITY(false, (false) && _GLF_IDENTITY(true, (false ? _GLF_FUZZED(true) : true)))))
            {
             _GLF_color = vec4(-410.591, 7.2, 97.92, 0.6);
            }
          }
        }
      }
    }
   else
    {
    }
  }
 while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))));
 float a = atan(_GLF_IDENTITY(p, clamp(p, p, p)).y, p.x) + (phase + t) * dir;
 float e = 0.198 + clamp(sin(_GLF_IDENTITY(_GLF_IDENTITY(a, clamp(a, a, _GLF_IDENTITY(a, (a) * 1.0))), clamp(_GLF_IDENTITY(a, (a) - 0.0), a, a)) * _GLF_IDENTITY(6.0, max(_GLF_IDENTITY(6.0, clamp(6.0, 6.0, 6.0)), 6.0))) * _GLF_IDENTITY(0.13, (0.13) + _GLF_ZERO(0.0, injectionSwitch.x)), 0.0, 0.1);
 if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (gl_FragCoord.x >= 0.0))))
  {
   if(_GLF_WRAPPED_IF_TRUE(true))
    {
     if(r < e)
      {
       if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (_GLF_IDENTITY(gl_FragCoord.y, _GLF_IDENTITY((false ? _GLF_FUZZED(pi) : gl_FragCoord.y), (_GLF_IDENTITY((false ? _GLF_FUZZED(pi) : gl_FragCoord.y), (true ? (false ? _GLF_FUZZED(pi) : gl_FragCoord.y) : _GLF_FUZZED(time)))) / 1.0)) >= 0.0))))
        {
         return vec4(GEAR_COLOR * (0.6 + e - r), 1.0);
        }
       else
        {
        }
       for(
           int _injected_loop_counter_50 = 1;
           _GLF_WRAPPED_LOOP(_injected_loop_counter_50 != 0);
           _injected_loop_counter_50 --
       )
        {
         if(_GLF_WRAPPED_IF_TRUE(true))
          {
           if(_GLF_DEAD(false))
            {
             if(_GLF_WRAPPED_IF_FALSE(false))
              {
              }
             else
              {
               _GLF_color = intBitsToFloat(ivec4(-34226, 55451, _GLF_IDENTITY(76562, min(76562, _GLF_IDENTITY(76562, min(_GLF_IDENTITY(76562, max(76562, 76562)), 76562)))), 71634));
              }
            }
          }
         else
          {
          }
        }
      }
     else
      {
       for(
           int _injected_loop_counter_3 = 0;
           _GLF_WRAPPED_LOOP(_GLF_IDENTITY(_GLF_IDENTITY(_GLF_IDENTITY(_GLF_IDENTITY(_injected_loop_counter_3, (_injected_loop_counter_3) / 1), (false ? _GLF_FUZZED(-89242) : _injected_loop_counter_3)) != int(_GLF_ONE(1.0, injectionSwitch.y)), true && (_GLF_IDENTITY(_injected_loop_counter_3, (false ? _GLF_FUZZED(-89242) : _injected_loop_counter_3)) != int(_GLF_ONE(1.0, injectionSwitch.y)))), true && (_GLF_IDENTITY(_GLF_IDENTITY(_injected_loop_counter_3, (_injected_loop_counter_3) / 1), (false ? _GLF_FUZZED(-89242) : _injected_loop_counter_3)) != int(_GLF_ONE(1.0, injectionSwitch.y)))));
           _injected_loop_counter_3 ++
       )
        {
         if(_GLF_WRAPPED_IF_FALSE(false))
          {
          }
         else
          {
           for(
               int _injected_loop_counter_1 = int(_GLF_ZERO(0.0, injectionSwitch.x));
               _GLF_WRAPPED_LOOP(_injected_loop_counter_1 < int(_GLF_ONE(1.0, _GLF_IDENTITY(injectionSwitch.y, max(injectionSwitch.y, _GLF_IDENTITY(injectionSwitch.y, max(injectionSwitch.y, injectionSwitch.y)))))));
               _injected_loop_counter_1 ++
           )
            {
             do
              {
               for(
                   int _injected_loop_counter_33 = 0;
                   _GLF_WRAPPED_LOOP(_injected_loop_counter_33 != 1);
                   _injected_loop_counter_33 ++
               )
                {
                 do
                  {
                   return vec4(0.0);
                  }
                 while(_GLF_WRAPPED_LOOP(false));
                }
              }
             while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))));
             if(_GLF_DEAD(_GLF_FALSE(false, (injectionSwitch.x > _GLF_IDENTITY(injectionSwitch, min(_GLF_IDENTITY(injectionSwitch, (injectionSwitch) * vec2(1.0, 1.0)), injectionSwitch)).y))))
              {
               _GLF_color = sign(vec4(3.2, -7.8, 1.6, 115.519));
              }
             for(
                 int _injected_loop_counter_43 = 1;
                 _GLF_WRAPPED_LOOP(_GLF_IDENTITY(_injected_loop_counter_43 != 0, true && (_injected_loop_counter_43 != 0)));
                 _injected_loop_counter_43 --
             )
              {
               for(
                   int _injected_loop_counter_51 = 0;
                   _GLF_WRAPPED_LOOP(_injected_loop_counter_51 != int(_GLF_ONE(1.0, injectionSwitch.y)));
                   _injected_loop_counter_51 ++
               )
                {
                 if(_GLF_DEAD(_GLF_FALSE(_GLF_IDENTITY(false, (false) && true), (_GLF_IDENTITY(injectionSwitch, (injectionSwitch) * vec2(1.0, 1.0)).x > _GLF_IDENTITY(injectionSwitch.y, min(injectionSwitch.y, _GLF_IDENTITY(injectionSwitch.y, (injectionSwitch.y) / _GLF_ONE(1.0, injectionSwitch.y))))))))
                  {
                   do
                    {
                     do
                      {
                       if(_GLF_WRAPPED_IF_TRUE(true))
                        {
                         _GLF_color = (vec4(397.967, 716.947, -4846.5766, -5.9) * mat4(4072.4623, -89.54, -1.5, -5.0, 8881.3536, -3618.5111, -78.88, -2347.2565, 49.47, 3.9, -44.68, -68.02, 30.65, -6558.8069, 397.151, 3811.4565));
                        }
                       else
                        {
                        }
                      }
                     while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))));
                    }
                   while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.y < _GLF_ZERO(0.0, injectionSwitch.x)))));
                  }
                }
               if(_GLF_DEAD(false))
                {
                 _GLF_color = asin(vec4(_GLF_IDENTITY(-2.8, max(-2.8, -2.8)), _GLF_IDENTITY(-7.6, (-7.6) - 0.0), 182.073, 9737.7270));
                }
              }
             if(_GLF_DEAD(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))))
              {
               do
                {
                 for(
                     int _injected_loop_counter_26 = 1;
                     _GLF_WRAPPED_LOOP(_injected_loop_counter_26 != 0);
                     _injected_loop_counter_26 --
                 )
                  {
                   do
                    {
                     do
                      {
                       for(
                           int _injected_loop_counter_11 = int(_GLF_ONE(1.0, injectionSwitch.y));
                           _GLF_WRAPPED_LOOP(_injected_loop_counter_11 != 0);
                           _injected_loop_counter_11 --
                       )
                        {
                         do
                          {
                           for(
                               int _injected_loop_counter_63 = _GLF_IDENTITY(int(_GLF_ONE(1.0, injectionSwitch.y)), (false ? _GLF_FUZZED(_injected_loop_counter_26) : _GLF_IDENTITY(int(_GLF_ONE(1.0, injectionSwitch.y)), (int(_GLF_ONE(1.0, injectionSwitch.y))) * 1)));
                               _GLF_WRAPPED_LOOP(_injected_loop_counter_63 > 0);
                               _injected_loop_counter_63 --
                           )
                            {
                             _GLF_color = (- vec4(-8.3, -606.514, 441.347, 304.831));
                            }
                          }
                         while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))));
                        }
                      }
                     while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))));
                    }
                   while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))));
                  }
                }
               while(_GLF_WRAPPED_LOOP(false));
              }
            }
          }
        }
       do
        {
         if(_GLF_WRAPPED_IF_TRUE(true))
          {
           if(_GLF_DEAD(false))
            {
             _GLF_color = uintBitsToFloat(uvec4(154093u, _GLF_IDENTITY(94372u, max(_GLF_IDENTITY(94372u, min(94372u, 94372u)), 94372u)), 1019u, 90720u));
            }
          }
         else
          {
          }
        }
       while(_GLF_WRAPPED_LOOP(false));
      }
     do
      {
       if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
        {
         if(_GLF_WRAPPED_IF_TRUE(true))
          {
           _GLF_color = mix(_GLF_IDENTITY(vec4(-5.9, -9.0, 9.8, -28.88), _GLF_IDENTITY(vec4(0.0, 0.0, 0.0, 0.0), vec4(_GLF_ZERO(0.0, injectionSwitch.x), _GLF_ZERO(0.0, injectionSwitch.x), _GLF_ZERO(0.0, injectionSwitch.x), _GLF_ZERO(0.0, injectionSwitch.x)) + (vec4(0.0, 0.0, 0.0, 0.0))) + (vec4(-5.9, -9.0, 9.8, -28.88))), _GLF_IDENTITY(vec4(-67.96, 5.4, -8060.4191, 29.48), clamp(_GLF_IDENTITY(vec4(-67.96, 5.4, -8060.4191, 29.48), clamp(vec4(-67.96, 5.4, -8060.4191, 29.48), vec4(-67.96, 5.4, -8060.4191, 29.48), _GLF_IDENTITY(vec4(-67.96, 5.4, -8060.4191, 29.48), max(vec4(-67.96, 5.4, -8060.4191, 29.48), vec4(-67.96, 5.4, -8060.4191, 29.48))))), vec4(-67.96, 5.4, -8060.4191, 29.48), vec4(-67.96, 5.4, -8060.4191, 29.48))), bvec4(false, false, false, true));
          }
         else
          {
          }
        }
      }
     while(_GLF_WRAPPED_LOOP(false));
    }
   else
    {
    }
  }
 else
  {
  }
 if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (_GLF_IDENTITY(gl_FragCoord.y < 0.0, (true ? gl_FragCoord.y < 0.0 : _GLF_FUZZED(true)))))))
  {
  }
 else
  {
   do
    {
     do
      {
       for(
           int _injected_loop_counter_77 = 1;
           _GLF_WRAPPED_LOOP(_injected_loop_counter_77 != int(_GLF_ZERO(0.0, injectionSwitch.x)));
           _injected_loop_counter_77 --
       )
        {
         if(_GLF_DEAD(false))
          {
           _GLF_color = vec4(9.5, -7181.3690, -619.833, 170.013);
          }
        }
       if(_GLF_WRAPPED_IF_FALSE(false))
        {
        }
       else
        {
         if(_GLF_DEAD(_GLF_IDENTITY(false, (false ? _GLF_FUZZED(true) : false))))
          {
           do
            {
             if(_GLF_WRAPPED_IF_FALSE(false))
              {
              }
             else
              {
               do
                {
                 if(_GLF_WRAPPED_IF_TRUE(true))
                  {
                   do
                    {
                     if(_GLF_DEAD(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))))
                      {
                       _GLF_color = vec4(84.85, -7499.6070, 78.03, -6088.9205);
                      }
                    }
                   while(_GLF_WRAPPED_LOOP(false));
                  }
                 else
                  {
                  }
                }
               while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))));
               do
                {
                 if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
                  {
                  }
                 else
                  {
                   do
                    {
                     for(
                         int _injected_loop_counter_78 = 1;
                         _GLF_WRAPPED_LOOP(_injected_loop_counter_78 > _GLF_IDENTITY(int(_GLF_ZERO(0.0, injectionSwitch.x)), (int(_GLF_ZERO(0.0, injectionSwitch.x))) - 0));
                         _injected_loop_counter_78 --
                     )
                      {
                       if(_GLF_WRAPPED_IF_FALSE(false))
                        {
                        }
                       else
                        {
                         do
                          {
                           if(_GLF_WRAPPED_IF_TRUE(true))
                            {
                             if(_GLF_WRAPPED_IF_TRUE(true))
                              {
                               if(_GLF_WRAPPED_IF_TRUE(true))
                                {
                                 if(_GLF_DEAD(_GLF_IDENTITY(false, (false ? _GLF_FUZZED(true) : false))))
                                  {
                                   _GLF_color = vec4(25.47, 7.3, 18.12, 79.71);
                                  }
                                 if(_GLF_WRAPPED_IF_FALSE(_GLF_IDENTITY(false, (true ? false : _GLF_FUZZED(true)))))
                                  {
                                  }
                                 else
                                  {
                                   for(
                                       int _injected_loop_counter_14 = int(_GLF_ONE(1.0, _GLF_IDENTITY(injectionSwitch, min(injectionSwitch, injectionSwitch)).y));
                                       _GLF_WRAPPED_LOOP(_injected_loop_counter_14 != _GLF_IDENTITY(int(_GLF_ZERO(0.0, injectionSwitch.x)), (_GLF_IDENTITY(int(_GLF_ZERO(0.0, injectionSwitch.x)), (int(_GLF_ZERO(0.0, injectionSwitch.x))) - 0)) / 1));
                                       _injected_loop_counter_14 --
                                   )
                                    {
                                     if(_GLF_DEAD(false))
                                      {
                                       _GLF_color = atan(vec4(2.0, -8.5, -2.8, -0.6), vec4(2.5, 7.0, -5723.1914, 4334.4168));
                                      }
                                     for(
                                         int _injected_loop_counter_5 = 0;
                                         _GLF_WRAPPED_LOOP(_injected_loop_counter_5 < _GLF_IDENTITY(int(_GLF_ONE(1.0, injectionSwitch.y)), (int(_GLF_ONE(1.0, injectionSwitch.y))) / 1));
                                         _injected_loop_counter_5 ++
                                     )
                                      {
                                       if(_GLF_WRAPPED_IF_FALSE(false))
                                        {
                                        }
                                       else
                                        {
                                         _GLF_color = vec4(-4.7, 18.64, 7158.0379, -4.0);
                                        }
                                      }
                                    }
                                  }
                                }
                               else
                                {
                                }
                               if(_GLF_DEAD(false))
                                {
                                 if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (injectionSwitch.x < injectionSwitch.y))))
                                  {
                                   _GLF_color = vec4(-5.3, 6.2, -600.843, 7.5);
                                  }
                                 else
                                  {
                                  }
                                }
                              }
                             else
                              {
                               if(_GLF_DEAD(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))))
                                {
                                 _GLF_color = vec4(9550.3619, 7.1, -54.40, -4973.7643);
                                }
                              }
                            }
                           else
                            {
                            }
                          }
                         while(_GLF_WRAPPED_LOOP(false));
                        }
                      }
                    }
                   while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))));
                  }
                }
               while(_GLF_WRAPPED_LOOP(false));
              }
            }
           while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))));
          }
        }
      }
     while(_GLF_WRAPPED_LOOP(false));
    }
   while(_GLF_WRAPPED_LOOP(false));
  }
}
vec4 gears(vec2 uv)
{
 vec4 c1 = gear(uv, 1.0, 0.0);
 vec4 c2 = gear(vec2(_GLF_IDENTITY(fract(uv.x + 0.5), min(_GLF_IDENTITY(fract(uv.x + 0.5), (true ? fract(uv.x + 0.5) : _GLF_FUZZED(-7.2))), fract(uv.x + 0.5))), uv.y), - 1.0, GEAR_PHASE);
 if(_GLF_DEAD(false))
  {
   for(
       int _injected_loop_counter_79 = 1;
       _GLF_WRAPPED_LOOP(_injected_loop_counter_79 != 0);
       _injected_loop_counter_79 --
   )
    {
     if(_GLF_WRAPPED_IF_FALSE(false))
      {
      }
     else
      {
       for(
           int _injected_loop_counter_52 = int(_GLF_ONE(1.0, injectionSwitch.y));
           _GLF_WRAPPED_LOOP(_injected_loop_counter_52 > 0);
           _injected_loop_counter_52 --
       )
        {
         _GLF_color = tan(vec4(-213.149, _GLF_IDENTITY(-1.0, min(_GLF_IDENTITY(-1.0, clamp(-1.0, -1.0, -1.0)), -1.0)), 1.2, 32.30));
        }
      }
    }
  }
 if(_GLF_DEAD(false))
  {
   if(_GLF_WRAPPED_IF_TRUE(true))
    {
     _GLF_color = acos(vec4(7.8, _GLF_IDENTITY(-5.9, (-5.9) / 1.0), -0.1, -12.31));
    }
   else
    {
    }
  }
 vec4 c3 = gear(vec2(uv.x, fract(_GLF_IDENTITY(_GLF_IDENTITY(uv.y, _GLF_IDENTITY(_GLF_IDENTITY(1.0 * (uv.y), (false ? _GLF_FUZZED(555.469) : 1.0 * (uv.y))), max(_GLF_IDENTITY(1.0, max(1.0, 1.0)) * (_GLF_IDENTITY(uv, (uv) / _GLF_IDENTITY(vec2(1.0, 1.0), (vec2(1.0, 1.0)) / vec2(1.0, 1.0))).y), 1.0 * (uv.y)))), min(uv.y, uv.y)) + _GLF_IDENTITY(0.5, (_GLF_IDENTITY(0.5, 1.0 * (0.5))) - _GLF_ZERO(0.0, injectionSwitch.x)))), - 1.0, GEAR_PHASE);
 if(_GLF_WRAPPED_IF_TRUE(true))
  {
   if(_GLF_WRAPPED_IF_FALSE(_GLF_IDENTITY(false, _GLF_IDENTITY((_GLF_IDENTITY(false, true && (_GLF_IDENTITY(false, true && (_GLF_IDENTITY(false, (false ? _GLF_FUZZED(false) : false))))))) || false, (true ? (_GLF_IDENTITY(false, true && (_GLF_IDENTITY(false, true && (_GLF_IDENTITY(false, (false ? _GLF_FUZZED(false) : false))))))) || false : _GLF_FUZZED(true))))))
    {
    }
   else
    {
     return c1 * c1.a + c2 * c2.a + c3 * c3.a;
    }
  }
 else
  {
  }
}
void main()
{
 if(_GLF_DEAD(false))
  {
   _GLF_color = vec4(1080.6780, -344.486, -0.1, -4.3);
  }
 if(_GLF_DEAD(false))
  {
   if(_GLF_DEAD(_GLF_FALSE(false, (_GLF_IDENTITY(gl_FragCoord, min(_GLF_IDENTITY(gl_FragCoord, clamp(gl_FragCoord, gl_FragCoord, gl_FragCoord)), gl_FragCoord)).y < 0.0))))
    {
     if(_GLF_DEAD(false))
      {
       _GLF_color = fract(clamp(_GLF_IDENTITY(vec4(-6023.1435, -15.03, -343.923, -8.1), (_GLF_IDENTITY(vec4(-6023.1435, -15.03, -343.923, -8.1), (vec4(-6023.1435, -15.03, -343.923, -8.1)) + vec4(0.0, 0.0, 0.0, 0.0))) * vec4(1.0, 1.0, 1.0, 1.0)), vec4(-9.7, -56.05, 9.5, 8.3), vec4(-6.7, 6.3, -66.88, -4.5)));
      }
     _GLF_color = vec4(-3.1, -4.5, 19.32, -6364.8092);
    }
   if(_GLF_DEAD(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))))
    {
     for(
         int _injected_loop_counter_64 = int(_GLF_ONE(1.0, injectionSwitch.y));
         _GLF_WRAPPED_LOOP(_injected_loop_counter_64 > int(_GLF_ZERO(0.0, injectionSwitch.x)));
         _injected_loop_counter_64 --
     )
      {
       for(
           int _injected_loop_counter_53 = int(_GLF_ONE(1.0, injectionSwitch.y));
           _GLF_WRAPPED_LOOP(_injected_loop_counter_53 > int(_GLF_ZERO(0.0, injectionSwitch.x)));
           _injected_loop_counter_53 --
       )
        {
         _GLF_color = vec4(294.140, 81.97, -6370.3628, -8149.2735);
        }
      }
    }
   if(_GLF_DEAD(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))))
    {
     _GLF_color = vec4(-42.65, -5659.8765, -86.62, 9.1);
    }
   _GLF_color = _GLF_IDENTITY(vec4(9.3, -8.2, 33.52, -744.425), (_GLF_IDENTITY(vec4(9.3, _GLF_IDENTITY(-8.2, max(-8.2, -8.2)), 33.52, -744.425), min(_GLF_IDENTITY(vec4(9.3, -8.2, _GLF_IDENTITY(33.52, max(33.52, 33.52)), -744.425), (_GLF_IDENTITY(vec4(9.3, -8.2, 33.52, -744.425), min(vec4(9.3, -8.2, 33.52, -744.425), vec4(9.3, -8.2, 33.52, -744.425)))) / vec4(1.0, 1.0, 1.0, _GLF_IDENTITY(1.0, 0.0 + (1.0)))), vec4(9.3, -8.2, 33.52, -744.425)))) + _GLF_IDENTITY(vec4(0.0, 0.0, 0.0, 0.0), (vec4(0.0, 0.0, 0.0, _GLF_IDENTITY(0.0, (_GLF_IDENTITY(0.0, (true ? 0.0 : _GLF_FUZZED(-3442.4993)))) + 0.0))) - vec4(_GLF_ZERO(0.0, injectionSwitch.x), _GLF_ZERO(0.0, injectionSwitch.x), _GLF_ZERO(0.0, injectionSwitch.x), _GLF_ZERO(0.0, injectionSwitch.x))));
  }
 if(_GLF_WRAPPED_IF_FALSE(false))
  {
  }
 else
  {
   if(_GLF_WRAPPED_IF_TRUE(true))
    {
     if(_GLF_WRAPPED_IF_FALSE(false))
      {
      }
     else
      {
       if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))))
        {
         if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (gl_FragCoord.x >= 0.0))))
          {
           _GLF_color = vec2(43.02, -6.7).ssst;
          }
         else
          {
          }
        }
      }
    }
   else
    {
    }
  }
 vec2 position = 2.0 * ((_GLF_IDENTITY(_GLF_IDENTITY(2.0, clamp(2.0, _GLF_IDENTITY(2.0, min(2.0, _GLF_IDENTITY(2.0, (false ? _GLF_FUZZED(time) : 2.0)))), 2.0)), min(2.0, _GLF_IDENTITY(2.0, clamp(2.0, 2.0, _GLF_IDENTITY(2.0, 0.0 + (2.0)))))) * gl_FragCoord.xy - resolution) / max(_GLF_IDENTITY(resolution.x, (resolution.x) * _GLF_ONE(1.0, injectionSwitch.y)), resolution.y));
 float p1 = 3.0;
 float p2 = 4.0;
 float u_corner = 2.0 * _GLF_IDENTITY(pi, max(_GLF_IDENTITY(pi, max(pi, _GLF_IDENTITY(pi, (false ? _GLF_FUZZED(time) : pi)))), pi)) * p2;
 float v_corner = log(256.0) * p1;
 float diag = sqrt(_GLF_IDENTITY(u_corner * u_corner, max(u_corner * u_corner, u_corner * u_corner)) + _GLF_IDENTITY(v_corner * v_corner, (_GLF_IDENTITY(v_corner * v_corner, (v_corner * v_corner) + 0.0)) - 0.0));
 float sin_a = v_corner / diag;
 do
  {
   for(
       int _injected_loop_counter_34 = 1;
       _GLF_WRAPPED_LOOP(_injected_loop_counter_34 > 0);
       _injected_loop_counter_34 --
   )
    {
     if(_GLF_DEAD(_GLF_FALSE(false, (_GLF_IDENTITY(gl_FragCoord.x, (gl_FragCoord.x) - 0.0) < 0.0))))
      {
       for(
           int _injected_loop_counter_54 = 1;
           _GLF_WRAPPED_LOOP(_injected_loop_counter_54 > 0);
           _injected_loop_counter_54 --
       )
        {
         if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (gl_FragCoord.y >= 0.0))))
          {
           _GLF_color = vec4(-265.446, -558.434, -98.74, 6463.9606);
          }
         else
          {
          }
        }
      }
    }
  }
 while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (_GLF_IDENTITY(gl_FragCoord, (gl_FragCoord) / vec4(_GLF_ONE(1.0, injectionSwitch.y), _GLF_ONE(1.0, _GLF_IDENTITY(injectionSwitch, max(_GLF_IDENTITY(injectionSwitch, clamp(injectionSwitch, injectionSwitch, injectionSwitch)), injectionSwitch)).y), _GLF_ONE(1.0, injectionSwitch.y), _GLF_ONE(1.0, injectionSwitch.y))).x < 0.0))));
 float cos_a = u_corner / _GLF_IDENTITY(diag, (diag) + 0.0);
 if(_GLF_WRAPPED_IF_FALSE(false))
  {
  }
 else
  {
   if(_GLF_DEAD(false))
    {
     if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
      {
      }
     else
      {
       for(
           int _injected_loop_counter_15 = int(_GLF_ZERO(0.0, injectionSwitch.x));
           _GLF_WRAPPED_LOOP(_injected_loop_counter_15 < 1);
           _injected_loop_counter_15 ++
       )
        {
         for(
             int _injected_loop_counter_65 = 1;
             _GLF_WRAPPED_LOOP(_injected_loop_counter_65 != _GLF_IDENTITY(0, max(0, 0)));
             _injected_loop_counter_65 --
         )
          {
           if(_GLF_DEAD(false))
            {
             for(
                 int _injected_loop_counter_55 = 1;
                 _GLF_WRAPPED_LOOP(_injected_loop_counter_55 != 0);
                 _injected_loop_counter_55 --
             )
              {
               for(
                   int _injected_loop_counter_27 = _GLF_IDENTITY(_GLF_IDENTITY(1, clamp(1, 1, 1)), (_GLF_IDENTITY(1, clamp(1, 1, 1))) * 1);
                   _GLF_WRAPPED_LOOP(_injected_loop_counter_27 > 0);
                   _injected_loop_counter_27 --
               )
                {
                 for(
                     int _injected_loop_counter_19 = 1;
                     _GLF_WRAPPED_LOOP(_GLF_IDENTITY(_injected_loop_counter_19, (true ? _injected_loop_counter_19 : _GLF_FUZZED(_GLF_IDENTITY(-47150, max(-47150, -47150))))) != _GLF_IDENTITY(0, max(0, 0)));
                     _injected_loop_counter_19 --
                 )
                  {
                   if(_GLF_DEAD(false))
                    {
                     for(
                         int _injected_loop_counter_66 = 1;
                         _GLF_WRAPPED_LOOP(_injected_loop_counter_66 > 0);
                         _injected_loop_counter_66 --
                     )
                      {
                       for(
                           int _injected_loop_counter_80 = 0;
                           _GLF_WRAPPED_LOOP(_injected_loop_counter_80 < int(_GLF_ONE(1.0, injectionSwitch.y)));
                           _injected_loop_counter_80 ++
                       )
                        {
                         _GLF_color = vec4(-4.1, -669.449, 1.2, -0.2);
                        }
                      }
                    }
                   for(
                       int _injected_loop_counter_67 = 0;
                       _GLF_WRAPPED_LOOP(_injected_loop_counter_67 < 1);
                       _injected_loop_counter_67 ++
                   )
                    {
                     if(_GLF_WRAPPED_IF_FALSE(false))
                      {
                      }
                     else
                      {
                       if(_GLF_WRAPPED_IF_FALSE(false))
                        {
                        }
                       else
                        {
                         do
                          {
                           _GLF_color = vec4(_GLF_IDENTITY(_GLF_IDENTITY(5710.2055, (5710.2055) + 0.0), min(5710.2055, 5710.2055)), 9.3, -8.0, -2.1);
                          }
                         while(_GLF_WRAPPED_LOOP(false));
                        }
                      }
                    }
                  }
                 do
                  {
                   do
                    {
                     for(
                         int _injected_loop_counter_56 = int(_GLF_ZERO(0.0, injectionSwitch.x));
                         _GLF_WRAPPED_LOOP(_injected_loop_counter_56 < int(_GLF_ONE(1.0, injectionSwitch.y)));
                         _injected_loop_counter_56 ++
                     )
                      {
                       if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
                        {
                         _GLF_color = vec4(-78.66, _GLF_IDENTITY(2.8, (_GLF_FALSE(false, (gl_FragCoord.x < 0.0)) ? _GLF_FUZZED(dot(injectionSwitch, resolution)) : _GLF_IDENTITY(2.8, max(2.8, 2.8)))), -891.948, 13.40);
                         if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))))
                          {
                           _GLF_color = vec4(-10.94, -9.1, _GLF_IDENTITY(23.87, max(23.87, 23.87)), -310.461);
                          }
                        }
                      }
                    }
                   while(_GLF_WRAPPED_LOOP(false));
                  }
                 while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))));
                }
              }
            }
          }
        }
      }
     if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (gl_FragCoord.x >= _GLF_ZERO(0.0, injectionSwitch.x)))))
      {
       do
        {
         for(
             int _injected_loop_counter_6 = 0;
             _GLF_WRAPPED_LOOP(_injected_loop_counter_6 < int(_GLF_ONE(1.0, injectionSwitch.y)));
             _injected_loop_counter_6 ++
         )
          {
           for(
               int _injected_loop_counter_28 = int(_GLF_ZERO(0.0, injectionSwitch.x));
               _GLF_WRAPPED_LOOP(_injected_loop_counter_28 != int(_GLF_ONE(1.0, injectionSwitch.y)));
               _injected_loop_counter_28 ++
           )
            {
             if(_GLF_DEAD(false))
              {
               if(_GLF_WRAPPED_IF_FALSE(false))
                {
                }
               else
                {
                 for(
                     int _injected_loop_counter_8 = 0;
                     _GLF_WRAPPED_LOOP(_injected_loop_counter_8 < 1);
                     _injected_loop_counter_8 ++
                 )
                  {
                   do
                    {
                     do
                      {
                       if(_GLF_WRAPPED_IF_TRUE(true))
                        {
                         if(_GLF_WRAPPED_IF_TRUE(true))
                          {
                           for(
                               int _injected_loop_counter_44 = int(_GLF_ONE(1.0, injectionSwitch.y));
                               _GLF_WRAPPED_LOOP(_injected_loop_counter_44 > 0);
                               _injected_loop_counter_44 --
                           )
                            {
                             do
                              {
                               if(_GLF_WRAPPED_IF_TRUE(true))
                                {
                                 if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (injectionSwitch.x < injectionSwitch.y))))
                                  {
                                   if(_GLF_DEAD(false))
                                    {
                                     _GLF_color = fract(vec4(-5.2, -262.213, -5.7, 7.4));
                                    }
                                   _GLF_color = vec4(-3198.5015, -25.58, -136.542, 25.65);
                                  }
                                 else
                                  {
                                  }
                                }
                               else
                                {
                                }
                              }
                             while(_GLF_WRAPPED_LOOP(false));
                            }
                          }
                         else
                          {
                          }
                        }
                       else
                        {
                        }
                      }
                     while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))));
                    }
                   while(_GLF_WRAPPED_LOOP(false));
                  }
                }
              }
            }
           if(_GLF_WRAPPED_IF_FALSE(false))
            {
            }
           else
            {
             for(
                 int _injected_loop_counter_81 = 0;
                 _GLF_WRAPPED_LOOP(_injected_loop_counter_81 < int(_GLF_ONE(1.0, injectionSwitch.y)));
                 _injected_loop_counter_81 ++
             )
              {
               _GLF_color = smoothstep(-46.82, 9049.9079, _GLF_IDENTITY(vec4(884.083, 0.1, -87.55, 9.5), min(_GLF_IDENTITY(vec4(884.083, 0.1, -87.55, 9.5), max(_GLF_IDENTITY(vec4(884.083, 0.1, -87.55, 9.5), (vec4(884.083, 0.1, -87.55, 9.5)) - vec4(0.0, 0.0, 0.0, 0.0)), _GLF_IDENTITY(vec4(884.083, 0.1, -87.55, 9.5), max(vec4(884.083, 0.1, -87.55, 9.5), vec4(884.083, 0.1, -87.55, 9.5))))), vec4(_GLF_IDENTITY(884.083, (884.083) - 0.0), 0.1, -87.55, 9.5))));
              }
            }
          }
        }
       while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))));
      }
     else
      {
      }
    }
  }
 float scale = _GLF_IDENTITY(diag, (_GLF_IDENTITY(diag, 1.0 * (_GLF_IDENTITY(diag, (diag) / 1.0)))) - _GLF_ZERO(0.0, injectionSwitch.x)) / 2.0 / pi;
 float offset = 1.0;
 vec2 p = clog(_GLF_IDENTITY(position, clamp(_GLF_IDENTITY(position, _GLF_IDENTITY((position) + vec2(0.0, 0.0), clamp((position) + vec2(0.0, 0.0), (position) + vec2(0.0, 0.0), _GLF_IDENTITY(_GLF_IDENTITY((position) + vec2(0.0, 0.0), clamp((position) + vec2(0.0, 0.0), (position) + vec2(0.0, 0.0), (position) + vec2(0.0, 0.0))), ((position) + vec2(0.0, _GLF_IDENTITY(0.0, (true ? 0.0 : _GLF_FUZZED(offset))))) / vec2(_GLF_IDENTITY(1.0, (_GLF_IDENTITY(_GLF_IDENTITY(true ? 1.0 : _GLF_FUZZED(_GLF_IDENTITY(cos_a, clamp(cos_a, _GLF_IDENTITY(cos_a, 0.0 + (cos_a)), cos_a))), (_GLF_IDENTITY(true ? 1.0 : _GLF_FUZZED(_GLF_IDENTITY(cos_a, clamp(cos_a, _GLF_IDENTITY(cos_a, 0.0 + (cos_a)), cos_a))), (true ? true ? 1.0 : _GLF_FUZZED(_GLF_IDENTITY(cos_a, clamp(cos_a, _GLF_IDENTITY(cos_a, 0.0 + (cos_a)), cos_a))) : _GLF_FUZZED(GEAR_PHASE)))) / 1.0), (true ? true ? 1.0 : _GLF_FUZZED(_GLF_IDENTITY(cos_a, clamp(cos_a, _GLF_IDENTITY(cos_a, 0.0 + (cos_a)), cos_a))) : _GLF_FUZZED(scale))))), 1.0))))), _GLF_IDENTITY(position, clamp(position, position, position)), position)) + vec2(offset, 0)) - _GLF_IDENTITY(clog(position + vec2(_GLF_IDENTITY(- offset, _GLF_IDENTITY(0.0, (_GLF_FALSE(false, (gl_FragCoord.y < 0.0)) ? _GLF_FUZZED(p2) : _GLF_IDENTITY(0.0, clamp(0.0, 0.0, 0.0)))) + (- offset)), 0)), (_GLF_IDENTITY(clog(position + vec2(_GLF_IDENTITY(- offset, 0.0 + (- offset)), 0)), vec2(1.0, 1.0) * (clog(position + vec2(_GLF_IDENTITY(- offset, 0.0 + (- _GLF_IDENTITY(offset, max(offset, offset)))), 0))))) / vec2(1.0, 1.0));
 vec2 rotated = vec2(p.x * _GLF_IDENTITY(cos_a, (cos_a) - 0.0) - p.y * sin_a, _GLF_IDENTITY(p, clamp(_GLF_IDENTITY(p, clamp(p, p, _GLF_IDENTITY(p, (p) * vec2(1.0, 1.0)))), p, p)).x * sin_a + _GLF_IDENTITY(_GLF_IDENTITY(p, max(_GLF_IDENTITY(p, min(p, p)), p)).y, max(_GLF_IDENTITY(p.y, min(p.y, p.y)), _GLF_IDENTITY(p.y, min(p.y, p.y)))) * cos_a);
 vec2 scaled = _GLF_IDENTITY(rotated * scale, min(_GLF_IDENTITY(rotated * scale, max(rotated * scale, rotated * scale)), rotated * scale)) / vec2(log(256.0), 2.0 * _GLF_IDENTITY(_GLF_IDENTITY(pi, clamp(_GLF_IDENTITY(pi, (pi) - 0.0), pi, _GLF_IDENTITY(pi, (pi) - 0.0))), (false ? _GLF_FUZZED(trunc(length(_GLF_color))) : pi)));
 for(
     int _injected_loop_counter_57 = 1;
     _GLF_WRAPPED_LOOP(_injected_loop_counter_57 != 0);
     _injected_loop_counter_57 --
 )
  {
   if(_GLF_WRAPPED_IF_FALSE(_GLF_IDENTITY(false, false || (false))))
    {
    }
   else
    {
     if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))))
      {
       if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (injectionSwitch.x < injectionSwitch.y))))
        {
         _GLF_color = _GLF_IDENTITY(vec4(_GLF_IDENTITY(60.81, (_GLF_IDENTITY(60.81, 0.0 + (60.81))) / 1.0), -95.51, 8.9, 7155.3484), clamp(_GLF_IDENTITY(vec4(60.81, -95.51, 8.9, 7155.3484), clamp(vec4(60.81, -95.51, 8.9, 7155.3484), _GLF_IDENTITY(vec4(60.81, -95.51, 8.9, 7155.3484), _GLF_IDENTITY((vec4(60.81, -95.51, 8.9, 7155.3484)) * vec4(1.0, 1.0, _GLF_IDENTITY(1.0, (1.0) + 0.0), 1.0), (_GLF_IDENTITY((vec4(60.81, -95.51, _GLF_IDENTITY(8.9, 0.0 + (8.9)), 7155.3484)), ((vec4(60.81, -95.51, _GLF_IDENTITY(8.9, 0.0 + (8.9)), 7155.3484))) * vec4(1.0, 1.0, 1.0, 1.0)) * vec4(1.0, 1.0, _GLF_IDENTITY(1.0, (1.0) + 0.0), 1.0)) + vec4(0.0, 0.0, 0.0, 0.0))), vec4(60.81, -95.51, 8.9, 7155.3484))), vec4(_GLF_IDENTITY(60.81, max(60.81, 60.81)), -95.51, 8.9, 7155.3484), vec4(60.81, -95.51, 8.9, 7155.3484)));
        }
       else
        {
        }
      }
    }
  }
 vec2 translated = scaled;
 if(_GLF_WRAPPED_IF_FALSE(false))
  {
  }
 else
  {
   if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (gl_FragCoord.y >= 0.0))))
    {
     for(
         int _injected_loop_counter_82 = 0;
         _GLF_WRAPPED_LOOP(_injected_loop_counter_82 < int(_GLF_ONE(1.0, injectionSwitch.y)));
         _injected_loop_counter_82 ++
     )
      {
       if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (injectionSwitch.x < injectionSwitch.y))))
        {
         if(_GLF_DEAD(false))
          {
           _GLF_color = exp2(vec4(-9723.2701, -2.3, 4.2, 79.42));
          }
         translated.x -= _GLF_IDENTITY((time * 0.05), (true ? _GLF_IDENTITY((time * 0.05), max((time * 0.05), (_GLF_IDENTITY(time * 0.05, (true ? time * 0.05 : _GLF_FUZZED(diag)))))) : _GLF_FUZZED(scale)));
        }
       else
        {
        }
      }
    }
   else
    {
     if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (injectionSwitch.x < injectionSwitch.y))))
      {
       if(_GLF_WRAPPED_IF_FALSE(false))
        {
        }
       else
        {
         if(_GLF_WRAPPED_IF_TRUE(_GLF_IDENTITY(true, (true ? true : _GLF_FUZZED(true)))))
          {
           if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))))
            {
             for(
                 int _injected_loop_counter_83 = 1;
                 _GLF_WRAPPED_LOOP(_injected_loop_counter_83 != 0);
                 _injected_loop_counter_83 --
             )
              {
               _GLF_color = vec4(-56.04, 883.720, 506.318, -8072.0285);
              }
            }
           for(
               int _injected_loop_counter_45 = 0;
               _GLF_WRAPPED_LOOP(_injected_loop_counter_45 != 1);
               _injected_loop_counter_45 ++
           )
            {
             do
              {
               if(_GLF_DEAD(false))
                {
                 _GLF_color = vec2(_GLF_IDENTITY(9122.2334, (true ? 9122.2334 : _GLF_FUZZED(u_corner))), -1040.5289).yxxy;
                }
              }
             while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))));
            }
          }
         else
          {
          }
        }
      }
     else
      {
      }
    }
   if(_GLF_DEAD(false))
    {
     for(
         int _injected_loop_counter_84 = _GLF_IDENTITY(0, clamp(_GLF_IDENTITY(0, (0) - 0), 0, 0));
         _GLF_WRAPPED_LOOP(_injected_loop_counter_84 < 1);
         _injected_loop_counter_84 ++
     )
      {
       _GLF_color = vec4(4.3, -2073.8546, -0.8, 59.26);
      }
    }
   if(_GLF_DEAD(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))))
    {
     _GLF_color = (vec2(-0.3, -6.1) * mat4x2(87.24, 9508.5865, 5705.1036, 1.0, 314.874, 30.61, 1519.7876, _GLF_IDENTITY(1249.4655, clamp(1249.4655, 1249.4655, 1249.4655))));
    }
  }
 if(_GLF_WRAPPED_IF_FALSE(false))
  {
  }
 else
  {
   do
    {
     for(
         int _injected_loop_counter_29 = int(_GLF_ONE(_GLF_IDENTITY(1.0, max(_GLF_IDENTITY(1.0, max(1.0, 1.0)), 1.0)), injectionSwitch.y));
         _GLF_WRAPPED_LOOP(_injected_loop_counter_29 > int(_GLF_ZERO(0.0, injectionSwitch.x)));
         _injected_loop_counter_29 --
     )
      {
       for(
           int _injected_loop_counter_16 = 1;
           _GLF_WRAPPED_LOOP(_injected_loop_counter_16 > int(_GLF_ZERO(0.0, injectionSwitch.x)));
           _injected_loop_counter_16 --
       )
        {
         for(
             int _injected_loop_counter_30 = 1;
             _GLF_WRAPPED_LOOP(_injected_loop_counter_30 != 0);
             _injected_loop_counter_30 --
         )
          {
           do
            {
             if(_GLF_WRAPPED_IF_FALSE(false))
              {
              }
             else
              {
               if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (injectionSwitch.x < injectionSwitch.y))))
                {
                 if(_GLF_WRAPPED_IF_TRUE(true))
                  {
                   if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
                    {
                     if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (injectionSwitch.x < injectionSwitch.y))))
                      {
                       if(_GLF_WRAPPED_IF_FALSE(false))
                        {
                        }
                       else
                        {
                         if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.x < _GLF_ZERO(0.0, injectionSwitch.x)))))
                          {
                           _GLF_color = vec4(sign(vec2(0.7, 24.13)), (482.627), -2347.3029);
                          }
                         _GLF_color = vec4(-5.5, 61.65, 465.804, -629.295);
                        }
                      }
                     else
                      {
                      }
                    }
                  }
                 else
                  {
                  }
                }
               else
                {
                }
              }
             do
              {
               for(
                   int _injected_loop_counter_4 = 1;
                   _GLF_WRAPPED_LOOP(_GLF_IDENTITY(_injected_loop_counter_4 != 0, (false ? _GLF_FUZZED(false) : _GLF_IDENTITY(_injected_loop_counter_4, max(_GLF_IDENTITY(_injected_loop_counter_4, min(_injected_loop_counter_4, _GLF_IDENTITY(_injected_loop_counter_4, min(_injected_loop_counter_4, _injected_loop_counter_4)))), _injected_loop_counter_4)) != 0)));
                   _injected_loop_counter_4 --
               )
                {
                 vec4 _GLF_outVarBackup_GLF_color;
                 if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))))
                  {
                  }
                 else
                  {
                   _GLF_outVarBackup_GLF_color = _GLF_color;
                   if(_GLF_DEAD(_GLF_FALSE(false, (_GLF_IDENTITY(_GLF_IDENTITY(injectionSwitch.x, (injectionSwitch.x) - 0.0) > injectionSwitch.y, (_GLF_IDENTITY(injectionSwitch.x, (injectionSwitch.x) - 0.0) > injectionSwitch.y) && true)))))
                    {
                     if(_GLF_WRAPPED_IF_TRUE(true))
                      {
                       _GLF_color = vec4(6965.8370, -695.522, -3.6, 6.0);
                      }
                     else
                      {
                      }
                    }
                  }
                 if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (_GLF_IDENTITY(injectionSwitch, (injectionSwitch) - vec2(0.0, 0.0)).x < injectionSwitch.y))))
                  {
                   _GLF_color = mat2x4(214.827, 7.6, 426.838, 3.9, -5215.0202, 4658.2278, -9745.2878, 973.019)[1];
                  }
                 else
                  {
                  }
                 for(
                     int _injected_loop_counter_2 = 0;
                     _GLF_WRAPPED_LOOP(_injected_loop_counter_2 != 1);
                     _injected_loop_counter_2 ++
                 )
                  {
                   for(
                       int _injected_loop_counter_68 = 1;
                       _GLF_WRAPPED_LOOP(_injected_loop_counter_68 > int(_GLF_ZERO(0.0, injectionSwitch.x)));
                       _injected_loop_counter_68 --
                   )
                    {
                     if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))))
                      {
                      }
                     else
                      {
                       if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (gl_FragCoord.y >= 0.0))))
                        {
                         if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(_GLF_IDENTITY(false, (false) || false), (injectionSwitch.x > _GLF_IDENTITY(injectionSwitch, clamp(_GLF_IDENTITY(injectionSwitch, min(injectionSwitch, injectionSwitch)), injectionSwitch, injectionSwitch)).y))))
                          {
                          }
                         else
                          {
                           if(_GLF_WRAPPED_IF_FALSE(false))
                            {
                            }
                           else
                            {
                             do
                              {
                               if(_GLF_WRAPPED_IF_FALSE(false))
                                {
                                }
                               else
                                {
                                 if(_GLF_WRAPPED_IF_TRUE(true))
                                  {
                                   if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (gl_FragCoord.x >= 0.0))))
                                    {
                                     if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (gl_FragCoord.y >= 0.0))))
                                      {
                                       if(_GLF_WRAPPED_IF_TRUE(true))
                                        {
                                         if(_GLF_DEAD(_GLF_IDENTITY(false, true && (false))))
                                          {
                                           if(_GLF_WRAPPED_IF_TRUE(true))
                                            {
                                             do
                                              {
                                               _GLF_color = vec4(859.919, -898.319, 6.1, -96.37);
                                              }
                                             while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))));
                                            }
                                           else
                                            {
                                            }
                                          }
                                        }
                                       else
                                        {
                                         if(_GLF_DEAD(_GLF_IDENTITY(false, (_GLF_IDENTITY(false, (false) || _GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y)))) && true)))
                                          {
                                           _GLF_color = vec4(-16.30, 7161.6497, 5.6, -6371.1442);
                                          }
                                        }
                                      }
                                     else
                                      {
                                      }
                                     if(_GLF_DEAD(_GLF_FALSE(_GLF_IDENTITY(false, (_GLF_IDENTITY(false, (false) && true)) || false), (_GLF_IDENTITY(gl_FragCoord.x, min(gl_FragCoord.x, gl_FragCoord.x)) < 0.0))))
                                      {
                                       if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (_GLF_IDENTITY(injectionSwitch.x, min(injectionSwitch.x, injectionSwitch.x)) > injectionSwitch.y))))
                                        {
                                        }
                                       else
                                        {
                                         if(_GLF_DEAD(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))))
                                          {
                                           for(
                                               int _injected_loop_counter_69 = 0;
                                               _GLF_WRAPPED_LOOP(_injected_loop_counter_69 != 1);
                                               _injected_loop_counter_69 ++
                                           )
                                            {
                                             _GLF_color = vec4(-76.13, 8.1, -87.63, -89.80);
                                            }
                                          }
                                         if(_GLF_DEAD(false))
                                          {
                                           for(
                                               int _injected_loop_counter_35 = int(_GLF_ZERO(0.0, injectionSwitch.x));
                                               _GLF_WRAPPED_LOOP(_injected_loop_counter_35 < 1);
                                               _injected_loop_counter_35 ++
                                           )
                                            {
                                             _GLF_color = vec4(-4.8, _GLF_IDENTITY(-45.97, clamp(-45.97, _GLF_IDENTITY(-45.97, clamp(-45.97, _GLF_IDENTITY(-45.97, (false ? _GLF_FUZZED(offset) : -45.97)), -45.97)), -45.97)), -74.10, -501.076);
                                            }
                                          }
                                        }
                                       if(_GLF_DEAD(false))
                                        {
                                         _GLF_color = sign((abs(_GLF_IDENTITY(-0.3, clamp(-0.3, _GLF_IDENTITY(-0.3, clamp(-0.3, -0.3, -0.3)), _GLF_IDENTITY(-0.3, (false ? _GLF_FUZZED(offset) : -0.3))))) / vec4(-6.6, 1972.0297, -0.7, 5.4)));
                                        }
                                       _GLF_color = vec4(3440.9678, -755.543, 6635.2937, -679.002);
                                      }
                                     do
                                      {
                                       do
                                        {
                                         _GLF_color = _GLF_outVarBackup_GLF_color;
                                         do
                                          {
                                           if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))))
                                            {
                                             if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
                                              {
                                              }
                                             else
                                              {
                                               if(_GLF_WRAPPED_IF_FALSE(false))
                                                {
                                                }
                                               else
                                                {
                                                 if(_GLF_DEAD(false))
                                                  {
                                                   _GLF_color = sin(vec4(6838.9850, 265.811, 4204.0715, 8306.5634));
                                                  }
                                                }
                                               _GLF_color = vec4(-58.42, 1.5, -609.422, -9.1);
                                              }
                                            }
                                          }
                                         while(_GLF_WRAPPED_LOOP(false));
                                        }
                                       while(_GLF_WRAPPED_LOOP(false));
                                      }
                                     while(_GLF_WRAPPED_LOOP(false));
                                    }
                                  }
                                 else
                                  {
                                  }
                                }
                              }
                             while(_GLF_WRAPPED_LOOP(false));
                            }
                          }
                        }
                       else
                        {
                        }
                      }
                    }
                  }
                 if(_GLF_DEAD(false))
                  {
                   if(_GLF_WRAPPED_IF_TRUE(true))
                    {
                     for(
                         int _injected_loop_counter_85 = int(_GLF_ONE(1.0, injectionSwitch.y));
                         _GLF_WRAPPED_LOOP(_injected_loop_counter_85 > 0);
                         _injected_loop_counter_85 --
                     )
                      {
                       if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
                        {
                         if(_GLF_WRAPPED_IF_FALSE(false))
                          {
                          }
                         else
                          {
                           for(
                               int _injected_loop_counter_58 = _GLF_IDENTITY(_GLF_IDENTITY(int(_GLF_ZERO(0.0, injectionSwitch.x)), (int(_GLF_ZERO(0.0, injectionSwitch.x))) / 1), (_GLF_IDENTITY(int(_GLF_ZERO(0.0, injectionSwitch.x)), (true ? int(_GLF_ZERO(0.0, injectionSwitch.x)) : _GLF_FUZZED(_injected_loop_counter_16)))) / 1);
                               _GLF_WRAPPED_LOOP(_injected_loop_counter_58 != 1);
                               _injected_loop_counter_58 ++
                           )
                            {
                             _GLF_color = vec4(38.48, -736.804, 3100.4435, 6825.7174);
                            }
                          }
                        }
                      }
                    }
                   else
                    {
                    }
                   do
                    {
                     if(_GLF_WRAPPED_IF_FALSE(false))
                      {
                       for(
                           int _injected_loop_counter_70 = 0;
                           _GLF_WRAPPED_LOOP(_injected_loop_counter_70 < int(_GLF_ONE(1.0, injectionSwitch.y)));
                           _injected_loop_counter_70 ++
                       )
                        {
                         do
                          {
                           if(_GLF_DEAD(false))
                            {
                             _GLF_color = round(vec4(996.665, -59.58, 28.16, 66.02));
                             if(_GLF_DEAD(false))
                              {
                               for(
                                   int _injected_loop_counter_86 = int(_GLF_ONE(1.0, injectionSwitch.y));
                                   _GLF_WRAPPED_LOOP(_injected_loop_counter_86 != 0);
                                   _injected_loop_counter_86 --
                               )
                                {
                                 _GLF_color = vec4(450.524, 42.73, -2951.1223, -1126.0242);
                                }
                              }
                            }
                          }
                         while(_GLF_WRAPPED_LOOP(false));
                        }
                      }
                     else
                      {
                       if(_GLF_DEAD(_GLF_IDENTITY(_GLF_FALSE(false, (gl_FragCoord.y < 0.0)), (_GLF_FALSE(false, (gl_FragCoord.y < 0.0))) && _GLF_TRUE(true, (gl_FragCoord.y >= 0.0)))))
                        {
                         for(
                             int _injected_loop_counter_71 = 0;
                             _GLF_WRAPPED_LOOP(_injected_loop_counter_71 < 1);
                             _injected_loop_counter_71 ++
                         )
                          {
                           if(_GLF_WRAPPED_IF_TRUE(true))
                            {
                             _GLF_color = (854.497 * vec4(-122.440, -9.5, _GLF_IDENTITY(_GLF_IDENTITY(805.936, min(805.936, 805.936)), (805.936) - _GLF_ZERO(0.0, injectionSwitch.x)), 0.4));
                            }
                           else
                            {
                            }
                          }
                        }
                       do
                        {
                         for(
                             int _injected_loop_counter_12 = int(_GLF_ONE(1.0, injectionSwitch.y));
                             _GLF_WRAPPED_LOOP(_injected_loop_counter_12 > 0);
                             _injected_loop_counter_12 --
                         )
                          {
                           if(_GLF_WRAPPED_IF_FALSE(false))
                            {
                            }
                           else
                            {
                             for(
                                 int _injected_loop_counter_46 = 1;
                                 _GLF_WRAPPED_LOOP(_injected_loop_counter_46 > int(_GLF_ZERO(0.0, injectionSwitch.x)));
                                 _injected_loop_counter_46 --
                             )
                              {
                               do
                                {
                                 do
                                  {
                                   if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(_GLF_IDENTITY(false, false || (_GLF_IDENTITY(false, (false) || false))), (gl_FragCoord.y < 0.0))))
                                    {
                                    }
                                   else
                                    {
                                     if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))))
                                      {
                                       if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
                                        {
                                         _GLF_color = clamp(atan(vec4(6.7, 6995.5869, _GLF_IDENTITY(-3.8, (false ? _GLF_FUZZED(float(_injected_loop_counter_12)) : _GLF_IDENTITY(-3.8, (false ? _GLF_FUZZED(time) : _GLF_IDENTITY(-3.8, (true ? -3.8 : _GLF_FUZZED(length(p)))))))), -1.9)), vec4(9696.0517, -4028.9538, -9.8, 5239.1319), cos(vec4(-9876.7544, -7.0, -395.693, -6.6)));
                                        }
                                      }
                                     else
                                      {
                                       if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (injectionSwitch.x < injectionSwitch.y))))
                                        {
                                         if(_GLF_WRAPPED_IF_FALSE(false))
                                          {
                                          }
                                         else
                                          {
                                           for(
                                               int _injected_loop_counter_87 = 0;
                                               _GLF_WRAPPED_LOOP(_injected_loop_counter_87 < 1);
                                               _injected_loop_counter_87 ++
                                           )
                                            {
                                             do
                                              {
                                               if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
                                                {
                                                 _GLF_color = (vec4(4002.4137, -97.40, 48.28, -9.0) * mat4(-2.5, -4.4, 57.73, 9297.8733, -3188.6601, -4.0, -6883.1461, 3.5, -6.0, -7.2, _GLF_IDENTITY(-7785.5220, (-7785.5220) / 1.0), -2440.9698, 3.4, -9050.3796, -2.0, 455.818));
                                                }
                                               _GLF_color = max(_GLF_IDENTITY(vec4(-9.6, -3.8, -692.267, -888.610), vec4(1.0, 1.0, 1.0, 1.0) * (vec4(-9.6, -3.8, -692.267, -888.610))), vec4(2.5, -4798.4455, 682.129, 1.0));
                                              }
                                             while(_GLF_WRAPPED_LOOP(false));
                                            }
                                           do
                                            {
                                             for(
                                                 int _injected_loop_counter_72 = 0;
                                                 _GLF_WRAPPED_LOOP(_injected_loop_counter_72 < 1);
                                                 _injected_loop_counter_72 ++
                                             )
                                              {
                                               if(_GLF_DEAD(false))
                                                {
                                                 _GLF_color = vec4(-6.6, -3.6, (fract(-754.627) / vec2(0.6, 5.1)));
                                                }
                                              }
                                            }
                                           while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (injectionSwitch.x > injectionSwitch.y))));
                                          }
                                        }
                                       else
                                        {
                                        }
                                      }
                                    }
                                  }
                                 while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))));
                                }
                               while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))));
                              }
                            }
                          }
                         for(
                             int _injected_loop_counter_59 = 0;
                             _GLF_WRAPPED_LOOP(_injected_loop_counter_59 != int(_GLF_ONE(1.0, injectionSwitch.y)));
                             _injected_loop_counter_59 ++
                         )
                          {
                           for(
                               int _injected_loop_counter_20 = int(_GLF_ONE(1.0, injectionSwitch.y));
                               _GLF_WRAPPED_LOOP(_injected_loop_counter_20 > 0);
                               _injected_loop_counter_20 --
                           )
                            {
                             if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))))
                              {
                               if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, (_GLF_IDENTITY(gl_FragCoord.y >= _GLF_IDENTITY(0.0, (0.0) * _GLF_ONE(1.0, injectionSwitch.y)), (_GLF_TRUE(true, (injectionSwitch.x < injectionSwitch.y)) ? _GLF_IDENTITY(gl_FragCoord.y >= 0.0, (true ? gl_FragCoord.y >= 0.0 : _GLF_FUZZED(false))) : _GLF_FUZZED((uvec3(155517u, 139706u, 41637u) == uvec3(156232u, 148832u, 151775u)))))))))
                                {
                                 do
                                  {
                                   for(
                                       int _injected_loop_counter_21 = 0;
                                       _GLF_WRAPPED_LOOP(_injected_loop_counter_21 < 1);
                                       _injected_loop_counter_21 ++
                                   )
                                    {
                                     if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.y < _GLF_ZERO(0.0, _GLF_IDENTITY(injectionSwitch, min(_GLF_IDENTITY(injectionSwitch, max(injectionSwitch, injectionSwitch)), _GLF_IDENTITY(injectionSwitch, max(injectionSwitch, _GLF_IDENTITY(injectionSwitch, min(injectionSwitch, injectionSwitch)))))).x)))))
                                      {
                                       _GLF_color = exp2(pow(vec4(9488.9530, -9.1, 97.87, 0.4), vec4(-24.39, 677.895, 130.068, 3.2)));
                                      }
                                     if(_GLF_WRAPPED_IF_TRUE(_GLF_TRUE(true, _GLF_IDENTITY((injectionSwitch.x < injectionSwitch.y), ((injectionSwitch.x < injectionSwitch.y)) && true))))
                                      {
                                       if(_GLF_WRAPPED_IF_FALSE(false))
                                        {
                                        }
                                       else
                                        {
                                         _GLF_color = vec4(9.8, -50.35, _GLF_IDENTITY(6.3, max(_GLF_IDENTITY(6.3, (_GLF_IDENTITY(6.3, 0.0 + (_GLF_IDENTITY(6.3, (false ? _GLF_FUZZED(u_corner) : 6.3))))) / 1.0), 6.3)), 8106.7994);
                                        }
                                      }
                                     else
                                      {
                                      }
                                    }
                                   if(_GLF_DEAD(false))
                                    {
                                     if(_GLF_WRAPPED_IF_TRUE(true))
                                      {
                                       _GLF_color = vec4(0.4, -61.64, -57.75, 272.333);
                                      }
                                     else
                                      {
                                      }
                                    }
                                  }
                                 while(_GLF_WRAPPED_LOOP(false));
                                }
                               else
                                {
                                }
                              }
                            }
                          }
                        }
                       while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (_GLF_IDENTITY(_GLF_IDENTITY(gl_FragCoord.y, min(gl_FragCoord.y, gl_FragCoord.y)) < 0.0, (false ? _GLF_FUZZED((uvec3(102375u, 97327u, 20876u) != uvec3(119227u, 33516u, 152532u))) : _GLF_IDENTITY(gl_FragCoord.y, min(gl_FragCoord.y, gl_FragCoord.y)) < 0.0))))));
                       if(_GLF_WRAPPED_IF_FALSE(false))
                        {
                        }
                       else
                        {
                         if(_GLF_DEAD(false))
                          {
                           _GLF_color = _GLF_IDENTITY(mix((vec4(2837.0346, -336.591, -9989.4664, 38.57)), vec4(8.1, -736.188, -38.08, 7.9), vec4(6767.4236, -4.4, 7.6, 29.56)), max(mix((vec4(2837.0346, -336.591, -9989.4664, 38.57)), vec4(8.1, -736.188, -38.08, 7.9), vec4(6767.4236, -4.4, 7.6, 29.56)), mix((vec4(2837.0346, -336.591, -9989.4664, 38.57)), vec4(8.1, -736.188, -38.08, 7.9), vec4(6767.4236, -4.4, 7.6, 29.56))));
                          }
                        }
                      }
                    }
                   while(_GLF_WRAPPED_LOOP(false));
                   if(_GLF_WRAPPED_IF_TRUE(true))
                    {
                     if(_GLF_DEAD(false))
                      {
                       do
                        {
                         for(
                             int _injected_loop_counter_73 = 1;
                             _GLF_WRAPPED_LOOP(_injected_loop_counter_73 != 0);
                             _injected_loop_counter_73 --
                         )
                          {
                           _GLF_color = reflect(vec4(-972.107, 6.9, -4.0, -0.8), vec4(-7.7, _GLF_IDENTITY(468.113, max(_GLF_IDENTITY(468.113, clamp(468.113, 468.113, _GLF_IDENTITY(468.113, (468.113) * 1.0))), _GLF_IDENTITY(468.113, max(468.113, 468.113)))), -2785.7106, -46.77));
                          }
                        }
                       while(_GLF_WRAPPED_LOOP(_GLF_IDENTITY(false, true && (false))));
                      }
                     if(_GLF_DEAD(false))
                      {
                       _GLF_color = vec4(3411.0269, -6268.9456, -86.55, 575.596);
                      }
                    }
                   else
                    {
                    }
                  }
                }
              }
             while(_GLF_WRAPPED_LOOP(false));
            }
           while(_GLF_WRAPPED_LOOP(false));
          }
         if(_GLF_DEAD(false))
          {
           _GLF_color = vec4(409.611, -939.819, 88.72, -9212.9714);
          }
        }
      }
    }
   while(_GLF_WRAPPED_LOOP(false));
  }
 if(_GLF_DEAD(false))
  {
   _GLF_color = vec4(1863.9767, -1.6, 99.71, 663.310);
  }
 vec4 gearCol = gears(fract(_GLF_IDENTITY(translated, (_GLF_IDENTITY(translated, min(translated, translated))) + vec2(_GLF_ZERO(_GLF_IDENTITY(0.0, (false ? _GLF_FUZZED(u_corner) : 0.0)), _GLF_IDENTITY(injectionSwitch, (injectionSwitch) - _GLF_IDENTITY(_GLF_IDENTITY(vec2(0.0, 0.0), vec2(1.0, 1.0) * (vec2(0.0, 0.0))), (_GLF_IDENTITY(vec2(0.0, 0.0), (vec2(0.0, 0.0)) - vec2(0.0, 0.0))) / vec2(1.0, 1.0))).x), _GLF_ZERO(0.0, _GLF_IDENTITY(injectionSwitch.x, (false ? _GLF_FUZZED(time) : _GLF_IDENTITY(injectionSwitch.x, max(injectionSwitch.x, injectionSwitch.x))))))) * 2.0)) * mix(0.4, 0.7, length(scaled));
 do
  {
   if(_GLF_WRAPPED_IF_FALSE(_GLF_FALSE(false, (_GLF_IDENTITY(gl_FragCoord.x, clamp(_GLF_IDENTITY(gl_FragCoord.x, min(_GLF_IDENTITY(gl_FragCoord.x, (true ? gl_FragCoord.x : _GLF_FUZZED(cos_a))), _GLF_IDENTITY(_GLF_IDENTITY(gl_FragCoord, min(gl_FragCoord, gl_FragCoord)).x, max(gl_FragCoord.x, gl_FragCoord.x)))), _GLF_IDENTITY(gl_FragCoord, vec4(1.0, 1.0, 1.0, 1.0) * (gl_FragCoord)).x, gl_FragCoord.x)) < _GLF_IDENTITY(0.0, (_GLF_FALSE(false, (gl_FragCoord.y < 0.0)) ? _GLF_FUZZED(u_corner) : _GLF_IDENTITY(0.0, min(_GLF_IDENTITY(0.0, max(_GLF_IDENTITY(0.0, _GLF_IDENTITY((false ? _GLF_FUZZED(pi) : 0.0), ((false ? _GLF_FUZZED(pi) : 0.0)) * 1.0)), 0.0)), 0.0))))))))
    {
     for(
         int _injected_loop_counter_88 = 1;
         _GLF_WRAPPED_LOOP(_injected_loop_counter_88 != int(_GLF_ZERO(0.0, injectionSwitch.x)));
         _injected_loop_counter_88 --
     )
      {
       for(
           int _injected_loop_counter_31 = 0;
           _GLF_WRAPPED_LOOP(_injected_loop_counter_31 != _GLF_IDENTITY(1, min(1, 1)));
           _injected_loop_counter_31 ++
       )
        {
         if(_GLF_DEAD(_GLF_FALSE(false, (gl_FragCoord.x < 0.0))))
          {
           if(_GLF_WRAPPED_IF_FALSE(false))
            {
            }
           else
            {
             for(
                 int _injected_loop_counter_74 = 1;
                 _GLF_WRAPPED_LOOP(_injected_loop_counter_74 > 0);
                 _injected_loop_counter_74 --
             )
              {
               do
                {
                 if(_GLF_WRAPPED_IF_FALSE(false))
                  {
                  }
                 else
                  {
                   for(
                       int _injected_loop_counter_89 = 0;
                       _GLF_WRAPPED_LOOP(_injected_loop_counter_89 < _GLF_IDENTITY(1, 1 * (1)));
                       _injected_loop_counter_89 ++
                   )
                    {
                     _GLF_color = mix(vec4(1154.7114, -589.723, -6.9, 5.7), vec4(7528.2244, -206.581, 35.02, -3.5), bvec4(false, true, false, true));
                    }
                  }
                }
               while(_GLF_WRAPPED_LOOP(_GLF_FALSE(false, (gl_FragCoord.y < 0.0))));
              }
            }
          }
        }
      }
    }
   else
    {
     for(
         int _injected_loop_counter_75 = 1;
         _GLF_WRAPPED_LOOP(_injected_loop_counter_75 > 0);
         _injected_loop_counter_75 --
     )
      {
       _GLF_color = gearCol * gearCol.a + vec4((1.0 - _GLF_IDENTITY(gearCol, (gearCol) / _GLF_IDENTITY(vec4(1.0, 1.0, _GLF_IDENTITY(1.0, (true ? 1.0 : _GLF_FUZZED(p2))), 1.0), clamp(vec4(1.0, 1.0, 1.0, 1.0), vec4(1.0, 1.0, 1.0, 1.0), vec4(1.0, 1.0, 1.0, 1.0)))).a) * mix(0.1, 0.2, length(_GLF_IDENTITY(scaled, (scaled) + _GLF_IDENTITY(vec2(0.0, 0.0), (vec2(0.0, 0.0)) + vec2(0.0, 0.0))))) * _GLF_IDENTITY(GEAR_COLOR, (GEAR_COLOR) - vec3(0.0, 0.0, 0.0)), 0.0);
      }
    }
  }
 while(_GLF_WRAPPED_LOOP(false));
}
