<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>AABB.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_a_a_b_b_8cpp</filename>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <includes id="_a_a_b_b_8h" name="AABB.h" local="yes" imported="no">AABB.h</includes>
    <includes id="_line_segment_8h" name="LineSegment.h" local="yes" imported="no">LineSegment.h</includes>
    <includes id="_line_8h" name="Line.h" local="yes" imported="no">Line.h</includes>
    <includes id="_ray_8h" name="Ray.h" local="yes" imported="no">Ray.h</includes>
    <includes id="_l_c_g_8h" name="LCG.h" local="yes" imported="no">LCG.h</includes>
    <includes id="_o_b_b_8h" name="OBB.h" local="yes" imported="no">OBB.h</includes>
    <includes id="_plane_8h" name="Plane.h" local="yes" imported="no">Plane.h</includes>
    <includes id="_sphere_8h" name="Sphere.h" local="yes" imported="no">Sphere.h</includes>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4_8h" name="float4.h" local="yes" imported="no">float4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">float4x4.h</includes>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Quat.h</includes>
    <includes id="_triangle_8h" name="Triangle.h" local="yes" imported="no">Triangle.h</includes>
    <member kind="function">
      <type>void</type>
      <name>AABBTransformAsAABB</name>
      <anchorfile>_a_a_b_b_8cpp.html</anchorfile>
      <anchor>ab28f0ebcb8da6ebc6f86bd233796bfac</anchor>
      <arglist>(AABB &amp;aabb, Matrix &amp;m)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IntersectRayAABB</name>
      <anchorfile>_a_a_b_b_8cpp.html</anchorfile>
      <anchor>adfe2050cd174b9455fe0305fdfa53fd9</anchor>
      <arglist>(const float3 &amp;rayPos, const float3 &amp;rayDir, const AABB &amp;aabb, float &amp;tNear, float &amp;tFar)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IntersectLineAABB</name>
      <anchorfile>_a_a_b_b_8cpp.html</anchorfile>
      <anchor>adfa0977a880d1e03f4566ec05b76eb6e</anchor>
      <arglist>(const float3 &amp;linePos, const float3 &amp;lineDir, const AABB &amp;aabb, float &amp;tNear, float &amp;tFar)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>AABB.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_a_a_b_b_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">AABB</class>
  </compound>
  <compound kind="file">
    <name>BitOps.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_bit_ops_8cpp</filename>
    <includes id="_bit_ops_8h" name="BitOps.h" local="yes" imported="no">BitOps.h</includes>
    <member kind="function">
      <type>u32</type>
      <name>BinaryStringToValue</name>
      <anchorfile>_bit_ops_8cpp.html</anchorfile>
      <anchor>afd610892f3361dd5c42b668be32065ef</anchor>
      <arglist>(const char *str)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>BitOps.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_bit_ops_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <class kind="class">LSBT</class>
    <class kind="class">BitMaskT</class>
    <member kind="function">
      <type>u32</type>
      <name>BinaryStringToValue</name>
      <anchorfile>_bit_ops_8h.html</anchorfile>
      <anchor>afd610892f3361dd5c42b668be32065ef</anchor>
      <arglist>(const char *str)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CountBitsSet</name>
      <anchorfile>_bit_ops_8h.html</anchorfile>
      <anchor>aa5d179f25ed00b64c03303a7e1cafc3a</anchor>
      <arglist>(u32 value)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>ExtractLSB</name>
      <anchorfile>_bit_ops_8h.html</anchorfile>
      <anchor>a9a2e478c30127254e49b9c2bd566678f</anchor>
      <arglist>(unsigned long *value)</arglist>
    </member>
    <member kind="function">
      <type>u32</type>
      <name>LSB</name>
      <anchorfile>_bit_ops_8h.html</anchorfile>
      <anchor>a17ca106ffeb0b2731d5d88aa653ca99a</anchor>
      <arglist>(u32 bits)</arglist>
    </member>
    <member kind="function">
      <type>u32</type>
      <name>BitMask</name>
      <anchorfile>_bit_ops_8h.html</anchorfile>
      <anchor>a18e648d1616fbe5862bfafca1f413e44</anchor>
      <arglist>(u32 pos, u32 bits)</arglist>
    </member>
    <member kind="function">
      <type>ResultType</type>
      <name>PackBits</name>
      <anchorfile>_bit_ops_8h.html</anchorfile>
      <anchor>aa709171804b3157ebde59f3f67028fa7</anchor>
      <arglist>(InputType a, InputType r, InputType g, InputType b)</arglist>
    </member>
    <member kind="function">
      <type>ResultType</type>
      <name>PackBits</name>
      <anchorfile>_bit_ops_8h.html</anchorfile>
      <anchor>a9fbe350a3cef536ab9030918fd40568c</anchor>
      <arglist>(int APos, int ABits, int RPos, int RBits, int GPos, int GBits, int BPos, int BBits, InputType a, InputType r, InputType g, InputType b)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ExtractBits</name>
      <anchorfile>_bit_ops_8h.html</anchorfile>
      <anchor>a376f5dc6cb3694552755367d0f8ccba6</anchor>
      <arglist>(ResultType &amp;out, const InputType &amp;in)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ExtractBits</name>
      <anchorfile>_bit_ops_8h.html</anchorfile>
      <anchor>a70922075ea505f17bbb7205c0b1f0a4c</anchor>
      <arglist>(int pos, int bits, ResultType &amp;out, const InputType &amp;in)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Circle.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_circle_8cpp</filename>
    <includes id="_circle_8h" name="Circle.h" local="yes" imported="no">Circle.h</includes>
    <includes id="_plane_8h" name="Plane.h" local="yes" imported="no">Plane.h</includes>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <includes id="_ray_8h" name="Ray.h" local="yes" imported="no">Ray.h</includes>
    <includes id="_line_segment_8h" name="LineSegment.h" local="yes" imported="no">LineSegment.h</includes>
    <includes id="_line_8h" name="Line.h" local="yes" imported="no">Line.h</includes>
  </compound>
  <compound kind="file">
    <name>Circle.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_circle_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">Circle</class>
  </compound>
  <compound kind="file">
    <name>Complex.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_complex_8h</filename>
    <class kind="class">Complex</class>
  </compound>
  <compound kind="file">
    <name>Cone.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_cone_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">Cone</class>
  </compound>
  <compound kind="file">
    <name>CoordinateAxisConvention.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_coordinate_axis_convention_8h</filename>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">float4x4.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
  </compound>
  <compound kind="file">
    <name>Cylinder.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_cylinder_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">Cylinder</class>
  </compound>
  <compound kind="file">
    <name>Ellipsoid.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_ellipsoid_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">Ellipsoid</class>
  </compound>
  <compound kind="file">
    <name>FixedPoint.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_fixed_point_8h</filename>
    <class kind="class">math::FixedPoint</class>
    <namespace>math</namespace>
    <member kind="function">
      <type>void</type>
      <name>Add</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a18a80f0cf2d7f8cd781cdf4015ef94e9</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;dst, const FixedPoint&lt; T, F2 &gt; &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Add</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>af1388b77882562b5ed1835e9ee314697</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;dst, const T &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Sub</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a832aad38f4d991280acb98fafcaa2898</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;dst, const FixedPoint&lt; T, F2 &gt; &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Sub</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a6cc3abe80fc18a9bbf9135ee433a5ea6</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;dst, const T &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mul</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a46d7e0bf17d2480593ba5b3d2d69c0b7</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;dst, const T &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>MulExtraFast</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a2ba7973d9dc3ff9679781f6df90c136f</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F2 &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>MulFast</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a10ff077aa8525420c0e9af9209acfd61</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>MulPrecise</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>aa8641fd334cbbaf6ccb14e3c1d0e5a11</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Div</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>aa735426ee1efa28cfa30c2fabf2121e7</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>DivExtraFast</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>aa19dc8641b25c14eb0e71b957ce6e644</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F2 &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt; &amp;</type>
      <name>operator+=</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a42107c70ddbbb8c61c184aee29cefdc9</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt; &amp;</type>
      <name>operator+=</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a369125fd6f6205362b876e66e09b6c10</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt;</type>
      <name>operator+</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a83cd3d0bff199a8fd7c53e8af03b2c68</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt;</type>
      <name>operator+</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>ad235c328e404e97537bc020f0f7f60bd</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt; &amp;</type>
      <name>operator-=</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>ae189df4785fd4bd7142cabd3731f961e</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt;</type>
      <name>operator-</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a3fe6d21dc03913ab3dca23d5d252caea</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt; &amp;</type>
      <name>operator*=</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a04e98020c5614269ada88fe8bbb16c1e</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt;</type>
      <name>operator*</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a85dd63be8a2fcbdd798ce028c01c5d83</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator&lt;</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>aea412d4cd40008d66ba9d03bad694f7e</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator&lt;</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>ab1a184f7d5631523691ec837133e9ae3</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>ad58d1aca0c00a465c8cb8c4f8814f317</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>abf4beebbcb47a69d81889c19ed114548</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a46de8314a7608e056a9c69dc4d208c7c</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>float2.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float2_8cpp</filename>
    <includes id="float2_8h" name="float2.h" local="yes" imported="no">float2.h</includes>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <class kind="class">SortByPolarAngle</class>
    <member kind="function">
      <type>float2</type>
      <name>operator*</name>
      <anchorfile>float2_8cpp.html</anchorfile>
      <anchor>ad0ba6718718e18c19c5ce57440c450ed</anchor>
      <arglist>(float scalar, const float2 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>operator/</name>
      <anchorfile>float2_8cpp.html</anchorfile>
      <anchor>acacbb404bc6b24d0c9469c49a6fcedbd</anchor>
      <arglist>(float scalar, const float2 &amp;rhs)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>float2.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float2_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <class kind="class">float2</class>
    <member kind="function">
      <type>float2</type>
      <name>operator*</name>
      <anchorfile>float2_8h.html</anchorfile>
      <anchor>ad0ba6718718e18c19c5ce57440c450ed</anchor>
      <arglist>(float scalar, const float2 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Dot</name>
      <anchorfile>float2_8h.html</anchorfile>
      <anchor>aac5070efed4e64b92b4961e730f332a8</anchor>
      <arglist>(const float2 &amp;a, const float2 &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Abs</name>
      <anchorfile>float2_8h.html</anchorfile>
      <anchor>a8eacde15853d997349f4387197444c48</anchor>
      <arglist>(const float2 &amp;a)</arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Min</name>
      <anchorfile>float2_8h.html</anchorfile>
      <anchor>a506d38bbcb707f99e45a922be8ff91ac</anchor>
      <arglist>(const float2 &amp;a, const float2 &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Max</name>
      <anchorfile>float2_8h.html</anchorfile>
      <anchor>aac8056d432f0fbab6e2189a5879f463a</anchor>
      <arglist>(const float2 &amp;a, const float2 &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Clamp</name>
      <anchorfile>float2_8h.html</anchorfile>
      <anchor>a38f74bbf3d199b47df903793cd3b9bac</anchor>
      <arglist>(const float2 &amp;a, float floor, float ceil)</arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Clamp</name>
      <anchorfile>float2_8h.html</anchorfile>
      <anchor>a1abd75044d6f4fa5d5e6b716e5f898e4</anchor>
      <arglist>(const float2 &amp;a, const float2 &amp;floor, const float2 &amp;ceil)</arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Clamp01</name>
      <anchorfile>float2_8h.html</anchorfile>
      <anchor>add2295c6cf6e5cfbd39efba5263e5dd5</anchor>
      <arglist>(const float2 &amp;a)</arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Lerp</name>
      <anchorfile>float2_8h.html</anchorfile>
      <anchor>a125b396d519d00f8f66c0bc7ffbf85d7</anchor>
      <arglist>(const float2 &amp;a, const float2 &amp;b, float t)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>float2x2.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float2x2_8cpp</filename>
  </compound>
  <compound kind="file">
    <name>float2x2.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float2x2_8h</filename>
  </compound>
  <compound kind="file">
    <name>float2x3.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float2x3_8cpp</filename>
  </compound>
  <compound kind="file">
    <name>float2x3.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float2x3_8h</filename>
  </compound>
  <compound kind="file">
    <name>float3.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float3_8cpp</filename>
    <includes id="float2_8h" name="float2.h" local="yes" imported="no">float2.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <includes id="float4_8h" name="float4.h" local="yes" imported="no">float4.h</includes>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <member kind="function">
      <type>float3</type>
      <name>operator*</name>
      <anchorfile>float3_8cpp.html</anchorfile>
      <anchor>a7ff11c486e2b12301dbbfa24436411ca</anchor>
      <arglist>(float scalar, const float3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>operator/</name>
      <anchorfile>float3_8cpp.html</anchorfile>
      <anchor>aa171af1f929f6e8e6c14c9c20b8b75cb</anchor>
      <arglist>(float scalar, const float3 &amp;rhs)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>float3.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float3_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <class kind="class">float3</class>
    <member kind="function">
      <type>float3</type>
      <name>operator*</name>
      <anchorfile>float3_8h.html</anchorfile>
      <anchor>a7ff11c486e2b12301dbbfa24436411ca</anchor>
      <arglist>(float scalar, const float3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Dot</name>
      <anchorfile>float3_8h.html</anchorfile>
      <anchor>a0c79bf266f2c14c24e808dddb7998a21</anchor>
      <arglist>(const float3 &amp;a, const float3 &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Cross</name>
      <anchorfile>float3_8h.html</anchorfile>
      <anchor>af8a3ffbe1290079315fbf1d47aec1cd0</anchor>
      <arglist>(const float3 &amp;a, const float3 &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Abs</name>
      <anchorfile>float3_8h.html</anchorfile>
      <anchor>a91b9adde01066d84bbb9a78bf6d15624</anchor>
      <arglist>(const float3 &amp;a)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Min</name>
      <anchorfile>float3_8h.html</anchorfile>
      <anchor>abe46bd975beefc40b0480c0ba9efac8d</anchor>
      <arglist>(const float3 &amp;a, const float3 &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Max</name>
      <anchorfile>float3_8h.html</anchorfile>
      <anchor>aaa6967be8e9afe4b2c6bc829ad5ce251</anchor>
      <arglist>(const float3 &amp;a, const float3 &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Clamp</name>
      <anchorfile>float3_8h.html</anchorfile>
      <anchor>a7b9d109454c09dbb326ee3ba2eb77272</anchor>
      <arglist>(const float3 &amp;a, float floor, float ceil)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Clamp</name>
      <anchorfile>float3_8h.html</anchorfile>
      <anchor>a56222978dbc72472c28869fbe03650e4</anchor>
      <arglist>(const float3 &amp;a, const float3 &amp;floor, const float3 &amp;ceil)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Clamp01</name>
      <anchorfile>float3_8h.html</anchorfile>
      <anchor>ac7a5156f654225267dd0983eddba6de5</anchor>
      <arglist>(const float3 &amp;a)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Lerp</name>
      <anchorfile>float3_8h.html</anchorfile>
      <anchor>a3723df263adeb3022311bda9aefb9304</anchor>
      <arglist>(const float3 &amp;a, const float3 &amp;b, float t)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>float3x3.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float3x3_8cpp</filename>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <includes id="float4_8h" name="float4.h" local="yes" imported="no">float4.h</includes>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">float4x4.h</includes>
    <includes id="_matrix_8inl" name="Matrix.inl" local="yes" imported="no">Matrix.inl</includes>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Quat.h</includes>
    <includes id="_transform_ops_8h" name="TransformOps.h" local="yes" imported="no">TransformOps.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>SKIPNUM</name>
      <anchorfile>float3x3_8cpp.html</anchorfile>
      <anchor>a9e41609db1f67e1406ee225355d9fbcd</anchor>
      <arglist>(val, skip)</arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>operator*</name>
      <anchorfile>float3x3_8cpp.html</anchorfile>
      <anchor>a89f8130b584992e6bad92d3065ce880d</anchor>
      <arglist>(const Quat &amp;lhs, const float3x3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>operator*</name>
      <anchorfile>float3x3_8cpp.html</anchorfile>
      <anchor>a694d8ef91d2b5395724f4a6f749f1dae</anchor>
      <arglist>(const float3 &amp;lhs, const float3x3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>operator*</name>
      <anchorfile>float3x3_8cpp.html</anchorfile>
      <anchor>a66eb3bc711841024410b33a2dc8352b8</anchor>
      <arglist>(const float4 &amp;lhs, const float3x3 &amp;rhs)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>float3x3.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float3x3_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="_matrix_proxy_8h" name="MatrixProxy.h" local="yes" imported="no">MatrixProxy.h</includes>
    <includes id="_coordinate_axis_convention_8h" name="CoordinateAxisConvention.h" local="yes" imported="no">CoordinateAxisConvention.h</includes>
    <class kind="class">float3x3</class>
    <member kind="function">
      <type>float3x3</type>
      <name>operator*</name>
      <anchorfile>float3x3_8h.html</anchorfile>
      <anchor>a89f8130b584992e6bad92d3065ce880d</anchor>
      <arglist>(const Quat &amp;lhs, const float3x3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>operator*</name>
      <anchorfile>float3x3_8h.html</anchorfile>
      <anchor>a694d8ef91d2b5395724f4a6f749f1dae</anchor>
      <arglist>(const float3 &amp;lhs, const float3x3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>operator*</name>
      <anchorfile>float3x3_8h.html</anchorfile>
      <anchor>a66eb3bc711841024410b33a2dc8352b8</anchor>
      <arglist>(const float4 &amp;lhs, const float3x3 &amp;rhs)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>float3x4.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float3x4_8cpp</filename>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <includes id="float4_8h" name="float4.h" local="yes" imported="no">float4.h</includes>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">float4x4.h</includes>
    <includes id="_matrix_8inl" name="Matrix.inl" local="yes" imported="no">Matrix.inl</includes>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Quat.h</includes>
    <includes id="_transform_ops_8h" name="TransformOps.h" local="yes" imported="no">TransformOps.h</includes>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>float3x4_8cpp.html</anchorfile>
      <anchor>a33e924c10949ea2b200501c65770a962</anchor>
      <arglist>(const Quat &amp;lhs, const float3x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>float3x4_8cpp.html</anchorfile>
      <anchor>a0bf8334fdc199ac333c77b499841e7a4</anchor>
      <arglist>(const float3x3 &amp;lhs, const float3x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>operator*</name>
      <anchorfile>float3x4_8cpp.html</anchorfile>
      <anchor>a9a77be136173c491e4aeae5c82cccb38</anchor>
      <arglist>(const float4 &amp;lhs, const float3x4 &amp;rhs)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>float3x4.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float3x4_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="_matrix_proxy_8h" name="MatrixProxy.h" local="yes" imported="no">MatrixProxy.h</includes>
    <includes id="_coordinate_axis_convention_8h" name="CoordinateAxisConvention.h" local="yes" imported="no">CoordinateAxisConvention.h</includes>
    <class kind="class">float3x4</class>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>float3x4_8h.html</anchorfile>
      <anchor>a33e924c10949ea2b200501c65770a962</anchor>
      <arglist>(const Quat &amp;lhs, const float3x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>float3x4_8h.html</anchorfile>
      <anchor>a0bf8334fdc199ac333c77b499841e7a4</anchor>
      <arglist>(const float3x3 &amp;lhs, const float3x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>operator*</name>
      <anchorfile>float3x4_8h.html</anchorfile>
      <anchor>a9a77be136173c491e4aeae5c82cccb38</anchor>
      <arglist>(const float4 &amp;lhs, const float3x4 &amp;rhs)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>float4.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float4_8cpp</filename>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <includes id="float4_8h" name="float4.h" local="yes" imported="no">float4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">float4x4.h</includes>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <member kind="function">
      <type>float4</type>
      <name>operator*</name>
      <anchorfile>float4_8cpp.html</anchorfile>
      <anchor>a260e8f76cce7929019f592f4a0499edd</anchor>
      <arglist>(float scalar, const float4 &amp;rhs)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>float4.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float4_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">float4</class>
    <member kind="function">
      <type>float4</type>
      <name>operator*</name>
      <anchorfile>float4_8h.html</anchorfile>
      <anchor>a260e8f76cce7929019f592f4a0499edd</anchor>
      <arglist>(float scalar, const float4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Dot3</name>
      <anchorfile>float4_8h.html</anchorfile>
      <anchor>ac0ccfdeba5d16a82d25f0c2f6ce8dbca</anchor>
      <arglist>(const float4 &amp;a, const float4 &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Dot4</name>
      <anchorfile>float4_8h.html</anchorfile>
      <anchor>a265579414e0c122c7a6cb5d1fbb9a19a</anchor>
      <arglist>(const float4 &amp;a, const float4 &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Cross3</name>
      <anchorfile>float4_8h.html</anchorfile>
      <anchor>ab3dbf8c6bd3b394dcfe99e335841b3f0</anchor>
      <arglist>(const float4 &amp;a, const float4 &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Abs</name>
      <anchorfile>float4_8h.html</anchorfile>
      <anchor>a31dbd1197deabc16ff3efa3a0a02635a</anchor>
      <arglist>(const float4 &amp;a)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Min</name>
      <anchorfile>float4_8h.html</anchorfile>
      <anchor>a44c59e48bab007d6533f65dd1a9bae86</anchor>
      <arglist>(const float4 &amp;a, const float4 &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Max</name>
      <anchorfile>float4_8h.html</anchorfile>
      <anchor>aab77eea1ff95fc9ee1a6dc9ff569af6b</anchor>
      <arglist>(const float4 &amp;a, const float4 &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Clamp</name>
      <anchorfile>float4_8h.html</anchorfile>
      <anchor>af44b572dcfad644709f265b87c0f8eaf</anchor>
      <arglist>(const float4 &amp;a, float floor, float ceil)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Clamp</name>
      <anchorfile>float4_8h.html</anchorfile>
      <anchor>a689272fa968fc9cc9b548748934b9dce</anchor>
      <arglist>(const float4 &amp;a, const float4 &amp;floor, const float4 &amp;ceil)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Clamp01</name>
      <anchorfile>float4_8h.html</anchorfile>
      <anchor>a016adb311f814000a0a00517d6826a15</anchor>
      <arglist>(const float4 &amp;a)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Lerp</name>
      <anchorfile>float4_8h.html</anchorfile>
      <anchor>a3a9fb766e9c020e30c613fdf5a2a756f</anchor>
      <arglist>(const float4 &amp;a, const float4 &amp;b, float t)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>float4x4.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float4x4_8cpp</filename>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <includes id="float4_8h" name="float4.h" local="yes" imported="no">float4.h</includes>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">float4x4.h</includes>
    <includes id="_matrix_8inl" name="Matrix.inl" local="yes" imported="no">Matrix.inl</includes>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Quat.h</includes>
    <includes id="_transform_ops_8h" name="TransformOps.h" local="yes" imported="no">TransformOps.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>SKIPNUM</name>
      <anchorfile>float4x4_8cpp.html</anchorfile>
      <anchor>a9e41609db1f67e1406ee225355d9fbcd</anchor>
      <arglist>(val, skip)</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>float4x4_8cpp.html</anchorfile>
      <anchor>a8fb0ad1dde9fe8cd8fef00d069b524e9</anchor>
      <arglist>(const Quat &amp;lhs, const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>float4x4_8cpp.html</anchorfile>
      <anchor>a01638cfed91347a9a1c7066ff19c3dd3</anchor>
      <arglist>(const float3x3 &amp;lhs, const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>float4x4_8cpp.html</anchorfile>
      <anchor>a391c6bd9fc269917e354f76791978163</anchor>
      <arglist>(const float3x4 &amp;lhs, const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>operator*</name>
      <anchorfile>float4x4_8cpp.html</anchorfile>
      <anchor>a95d417a391e2ee4a14c15e0436265b14</anchor>
      <arglist>(const float4 &amp;lhs, const float4x4 &amp;rhs)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>float4x4.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>float4x4_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="_matrix_proxy_8h" name="MatrixProxy.h" local="yes" imported="no">MatrixProxy.h</includes>
    <includes id="_coordinate_axis_convention_8h" name="CoordinateAxisConvention.h" local="yes" imported="no">CoordinateAxisConvention.h</includes>
    <class kind="class">float4x4</class>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>float4x4_8h.html</anchorfile>
      <anchor>a8fb0ad1dde9fe8cd8fef00d069b524e9</anchor>
      <arglist>(const Quat &amp;lhs, const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>float4x4_8h.html</anchorfile>
      <anchor>a391c6bd9fc269917e354f76791978163</anchor>
      <arglist>(const float3x4 &amp;lhs, const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>float4x4_8h.html</anchorfile>
      <anchor>a01638cfed91347a9a1c7066ff19c3dd3</anchor>
      <arglist>(const float3x3 &amp;lhs, const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>operator*</name>
      <anchorfile>float4x4_8h.html</anchorfile>
      <anchor>a95d417a391e2ee4a14c15e0436265b14</anchor>
      <arglist>(const float4 &amp;lhs, const float4x4 &amp;rhs)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>FloatCmp.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_float_cmp_8h</filename>
    <member kind="function">
      <type>bool</type>
      <name>Equal</name>
      <anchorfile>_float_cmp_8h.html</anchorfile>
      <anchor>a8957f554de5f9473096c5f619e470189</anchor>
      <arglist>(double a, double b, double epsilon=1e-6)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Frustum.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_frustum_8cpp</filename>
    <includes id="_a_a_b_b_8h" name="AABB.h" local="yes" imported="no">AABB.h</includes>
    <includes id="_circle_8h" name="Circle.h" local="yes" imported="no">Circle.h</includes>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <includes id="_frustum_8h" name="Frustum.h" local="yes" imported="no">Frustum.h</includes>
    <includes id="_plane_8h" name="Plane.h" local="yes" imported="no">Plane.h</includes>
    <includes id="_line_8h" name="Line.h" local="yes" imported="no">Line.h</includes>
    <includes id="_o_b_b_8h" name="OBB.h" local="yes" imported="no">OBB.h</includes>
    <includes id="_ray_8h" name="Ray.h" local="yes" imported="no">Ray.h</includes>
    <includes id="_sphere_8h" name="Sphere.h" local="yes" imported="no">Sphere.h</includes>
    <includes id="_triangle_8h" name="Triangle.h" local="yes" imported="no">Triangle.h</includes>
    <includes id="_line_segment_8h" name="LineSegment.h" local="yes" imported="no">LineSegment.h</includes>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4_8h" name="float4.h" local="yes" imported="no">float4.h</includes>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Quat.h</includes>
  </compound>
  <compound kind="file">
    <name>Frustum.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_frustum_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">Frustum</class>
  </compound>
  <compound kind="file">
    <name>HitInfo.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_hit_info_8h</filename>
    <class kind="struct">HitInfo</class>
  </compound>
  <compound kind="file">
    <name>LCG.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_l_c_g_8cpp</filename>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <includes id="_l_c_g_8h" name="LCG.h" local="yes" imported="no">LCG.h</includes>
  </compound>
  <compound kind="file">
    <name>LCG.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_l_c_g_8h</filename>
    <class kind="class">LCG</class>
  </compound>
  <compound kind="file">
    <name>Line.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_line_8cpp</filename>
    <includes id="_line_8h" name="Line.h" local="yes" imported="no">Line.h</includes>
    <includes id="_ray_8h" name="Ray.h" local="yes" imported="no">Ray.h</includes>
    <includes id="_line_segment_8h" name="LineSegment.h" local="yes" imported="no">LineSegment.h</includes>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">float4x4.h</includes>
    <includes id="_o_b_b_8h" name="OBB.h" local="yes" imported="no">OBB.h</includes>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Quat.h</includes>
    <includes id="_triangle_8h" name="Triangle.h" local="yes" imported="no">Triangle.h</includes>
    <includes id="_plane_8h" name="Plane.h" local="yes" imported="no">Plane.h</includes>
    <includes id="_sphere_8h" name="Sphere.h" local="yes" imported="no">Sphere.h</includes>
    <includes id="_a_a_b_b_8h" name="AABB.h" local="yes" imported="no">AABB.h</includes>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <member kind="function">
      <type>float</type>
      <name>Dmnop</name>
      <anchorfile>_line_8cpp.html</anchorfile>
      <anchor>a75646fced7b8e11b1acc00ac272ae07b</anchor>
      <arglist>(const float3 *v, int m, int n, int o, int p)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>LineLine</name>
      <anchorfile>_line_8cpp.html</anchorfile>
      <anchor>afdc98b11a60d9dd402c05274caa908de</anchor>
      <arglist>(float3 start0, float3 end0, float3 start1, float3 end1, float *d, float *d2)</arglist>
    </member>
    <member kind="function">
      <type>Line</type>
      <name>operator*</name>
      <anchorfile>_line_8cpp.html</anchorfile>
      <anchor>ae08bb802ec375e48ad0bfceb42602673</anchor>
      <arglist>(const float3x3 &amp;transform, const Line &amp;l)</arglist>
    </member>
    <member kind="function">
      <type>Line</type>
      <name>operator*</name>
      <anchorfile>_line_8cpp.html</anchorfile>
      <anchor>abada7c22d780dbf6bc8e6ecda595574f</anchor>
      <arglist>(const float3x4 &amp;transform, const Line &amp;l)</arglist>
    </member>
    <member kind="function">
      <type>Line</type>
      <name>operator*</name>
      <anchorfile>_line_8cpp.html</anchorfile>
      <anchor>a71e3d3ae507be4e38e95ffa540447532</anchor>
      <arglist>(const float4x4 &amp;transform, const Line &amp;l)</arglist>
    </member>
    <member kind="function">
      <type>Line</type>
      <name>operator*</name>
      <anchorfile>_line_8cpp.html</anchorfile>
      <anchor>a30dba77009fd41ce9c0d6d5ec9109679</anchor>
      <arglist>(const Quat &amp;transform, const Line &amp;l)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Line.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_line_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">Line</class>
    <member kind="function">
      <type>Line</type>
      <name>operator*</name>
      <anchorfile>_line_8h.html</anchorfile>
      <anchor>ae00fbdf9a3478ab08d8acae669599558</anchor>
      <arglist>(const float3x3 &amp;transform, const Line &amp;line)</arglist>
    </member>
    <member kind="function">
      <type>Line</type>
      <name>operator*</name>
      <anchorfile>_line_8h.html</anchorfile>
      <anchor>adaa72905ec51e2de761d34be800e5c17</anchor>
      <arglist>(const float3x4 &amp;transform, const Line &amp;line)</arglist>
    </member>
    <member kind="function">
      <type>Line</type>
      <name>operator*</name>
      <anchorfile>_line_8h.html</anchorfile>
      <anchor>a5af9a25315a1b67b12f53faa66bb8bc2</anchor>
      <arglist>(const float4x4 &amp;transform, const Line &amp;line)</arglist>
    </member>
    <member kind="function">
      <type>Line</type>
      <name>operator*</name>
      <anchorfile>_line_8h.html</anchorfile>
      <anchor>ad784d9e57eaeed4c4d4980a859a5cf52</anchor>
      <arglist>(const Quat &amp;transform, const Line &amp;line)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Dmnop</name>
      <anchorfile>_line_8h.html</anchorfile>
      <anchor>a75646fced7b8e11b1acc00ac272ae07b</anchor>
      <arglist>(const float3 *v, int m, int n, int o, int p)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>LineLine</name>
      <anchorfile>_line_8h.html</anchorfile>
      <anchor>afdc98b11a60d9dd402c05274caa908de</anchor>
      <arglist>(float3 start0, float3 end0, float3 start1, float3 end1, float *d, float *d2)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>LineSegment.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_line_segment_8cpp</filename>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <includes id="_a_a_b_b_8h" name="AABB.h" local="yes" imported="no">AABB.h</includes>
    <includes id="_line_segment_8h" name="LineSegment.h" local="yes" imported="no">LineSegment.h</includes>
    <includes id="_ray_8h" name="Ray.h" local="yes" imported="no">Ray.h</includes>
    <includes id="_line_8h" name="Line.h" local="yes" imported="no">Line.h</includes>
    <includes id="_plane_8h" name="Plane.h" local="yes" imported="no">Plane.h</includes>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">float4x4.h</includes>
    <includes id="_o_b_b_8h" name="OBB.h" local="yes" imported="no">OBB.h</includes>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Quat.h</includes>
    <includes id="_sphere_8h" name="Sphere.h" local="yes" imported="no">Sphere.h</includes>
    <includes id="_triangle_8h" name="Triangle.h" local="yes" imported="no">Triangle.h</includes>
    <member kind="function">
      <type>LineSegment</type>
      <name>operator*</name>
      <anchorfile>_line_segment_8cpp.html</anchorfile>
      <anchor>a3ac78332d11a81eeb147bb349d668ef7</anchor>
      <arglist>(const float3x3 &amp;transform, const LineSegment &amp;l)</arglist>
    </member>
    <member kind="function">
      <type>LineSegment</type>
      <name>operator*</name>
      <anchorfile>_line_segment_8cpp.html</anchorfile>
      <anchor>adf08902e9f5e0da129e548757fd2bb92</anchor>
      <arglist>(const float3x4 &amp;transform, const LineSegment &amp;l)</arglist>
    </member>
    <member kind="function">
      <type>LineSegment</type>
      <name>operator*</name>
      <anchorfile>_line_segment_8cpp.html</anchorfile>
      <anchor>a9563f30f52fdfdd9dfb9f75edbebd017</anchor>
      <arglist>(const float4x4 &amp;transform, const LineSegment &amp;l)</arglist>
    </member>
    <member kind="function">
      <type>LineSegment</type>
      <name>operator*</name>
      <anchorfile>_line_segment_8cpp.html</anchorfile>
      <anchor>ac3204c2827bb2a1834af1bb067313d5b</anchor>
      <arglist>(const Quat &amp;transform, const LineSegment &amp;l)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>LineSegment.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_line_segment_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">LineSegment</class>
    <member kind="function">
      <type>LineSegment</type>
      <name>operator*</name>
      <anchorfile>_line_segment_8h.html</anchorfile>
      <anchor>ae2ab5883c2b362a7d2df695bc7e69c5e</anchor>
      <arglist>(const float3x3 &amp;transform, const LineSegment &amp;line)</arglist>
    </member>
    <member kind="function">
      <type>LineSegment</type>
      <name>operator*</name>
      <anchorfile>_line_segment_8h.html</anchorfile>
      <anchor>a91e83d7a04120f8525f1f1b1d50a9946</anchor>
      <arglist>(const float3x4 &amp;transform, const LineSegment &amp;line)</arglist>
    </member>
    <member kind="function">
      <type>LineSegment</type>
      <name>operator*</name>
      <anchorfile>_line_segment_8h.html</anchorfile>
      <anchor>af98262883a21f8416ffba0b16a75f481</anchor>
      <arglist>(const float4x4 &amp;transform, const LineSegment &amp;line)</arglist>
    </member>
    <member kind="function">
      <type>LineSegment</type>
      <name>operator*</name>
      <anchorfile>_line_segment_8h.html</anchorfile>
      <anchor>a683d0db25952a99590ed83bd2b3d8e67</anchor>
      <arglist>(const Quat &amp;transform, const LineSegment &amp;line)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>MathConstants.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_math_constants_8h</filename>
    <member kind="variable">
      <type>const float</type>
      <name>recipSqrt2Pi</name>
      <anchorfile>_math_constants_8h.html</anchorfile>
      <anchor>a209557842a81c7538dbdd5fa14e856eb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const float</type>
      <name>cos1</name>
      <anchorfile>_math_constants_8h.html</anchorfile>
      <anchor>aee5de832dab293296a45148ba2af97c7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const float</type>
      <name>ln2</name>
      <anchorfile>_math_constants_8h.html</anchorfile>
      <anchor>ae83e5030253f51f2b5370310d392abfa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const float</type>
      <name>cbrt3</name>
      <anchorfile>_math_constants_8h.html</anchorfile>
      <anchor>a6d9e40005062366deea75c1c3ba6088c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const float</type>
      <name>recipLn2</name>
      <anchorfile>_math_constants_8h.html</anchorfile>
      <anchor>a9d79a83f4a2384514688295c8a8609b2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const float</type>
      <name>phi</name>
      <anchorfile>_math_constants_8h.html</anchorfile>
      <anchor>a1e1dcf0e9e30cecc4be610ccedf39162</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const float</type>
      <name>ln10</name>
      <anchorfile>_math_constants_8h.html</anchorfile>
      <anchor>a0457d6b5e39020935028c4459a8b9c20</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const float</type>
      <name>e</name>
      <anchorfile>_math_constants_8h.html</anchorfile>
      <anchor>a4225a7492f7d468ad5b985282c8ee7b1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const float</type>
      <name>pi</name>
      <anchorfile>_math_constants_8h.html</anchorfile>
      <anchor>abce8f0db8a5282e441988c8d2e73f79e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const float</type>
      <name>e2</name>
      <anchorfile>_math_constants_8h.html</anchorfile>
      <anchor>adb7ce5cfc534576cd12efa607dc7e3b1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>MathFunc.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_math_func_8cpp</filename>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <member kind="function">
      <type>void</type>
      <name>SetMathBreakOnAssume</name>
      <anchorfile>_math_func_8cpp.html</anchorfile>
      <anchor>a66b155c2afacb1202344006749b1d616</anchor>
      <arglist>(bool isEnabled)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>MathBreakOnAssume</name>
      <anchorfile>_math_func_8cpp.html</anchorfile>
      <anchor>a7ae2d272b0387b15780bd65968c8a956</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Factorial</name>
      <anchorfile>_math_func_8cpp.html</anchorfile>
      <anchor>a0e7da4d7a69c5b5afb707807aa177017</anchor>
      <arglist>(int n)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CombinatorialRec</name>
      <anchorfile>_math_func_8cpp.html</anchorfile>
      <anchor>a7dace71cdf9b9e7b3297a6cec5bc93ee</anchor>
      <arglist>(int n, int k)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CombinatorialTab</name>
      <anchorfile>_math_func_8cpp.html</anchorfile>
      <anchor>ab04af431a63f29c0e20f5a67c15f25b6</anchor>
      <arglist>(int n, int k)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>PowUInt</name>
      <anchorfile>_math_func_8cpp.html</anchorfile>
      <anchor>aa5efeff5a30fb8c0f27b98ce1dae7971</anchor>
      <arglist>(float base, u32 exponent)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>PowInt</name>
      <anchorfile>_math_func_8cpp.html</anchorfile>
      <anchor>a7d4f6ec790e5c466fcda5bc3aa53b970</anchor>
      <arglist>(float base, int exponent)</arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>mathBreakOnAssume</name>
      <anchorfile>_math_func_8cpp.html</anchorfile>
      <anchor>a5a45b270036ff608f1f207d487f23b5a</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>MathFunc.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_math_func_8h</filename>
    <includes id="_math_constants_8h" name="MathConstants.h" local="yes" imported="no">MathConstants.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">PowIntT</class>
    <class kind="class">FactorialT</class>
    <class kind="class">CombinatorialT</class>
    <member kind="define">
      <type>#define</type>
      <name>assume</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a32cf720adc95c26d4606894723412ed6</anchor>
      <arglist>(x)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DOT2</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a34d9834116303b2239a9c47cc42eef8f</anchor>
      <arglist>(v1, v2)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DOT3</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a6c5ac2caca1bcc3fc310eb18d7764d4a</anchor>
      <arglist>(v1, v2)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>ABSDOT3</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>acfd6d0735ddb5e088fe9f39f0bdfba69</anchor>
      <arglist>(v1, v2)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DOT3_xyz</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>abc2bd907f1eafc827e03d3c036545957</anchor>
      <arglist>(v1, x, y, z)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DOT3STRIDED</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>ae5e18fa5e87f2a78a0f5f972ffee3279</anchor>
      <arglist>(v1, v2, stride)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DOT4</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a29a4af063591f55216c65b59d75d3dde</anchor>
      <arglist>(v1, v2)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DOT4STRIDED</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a50716388e9d680ed0050c7995bf65d9e</anchor>
      <arglist>(v1, v2, stride)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DOT4POS</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a7dbeae6ed8941fb9f800f01d4e15aa6d</anchor>
      <arglist>(vec4D, vecPos)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DOT4POS_xyz</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a403586e5f91731e0c871ab0d89972379</anchor>
      <arglist>(vec4D, x, y, z)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DOT4DIR</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>ac8233b137621c5897ec3bb41f7142259</anchor>
      <arglist>(vec4D, vecDir)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DOT4DIR_xyz</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a5582d5131d2192727ae2cfde5d23a5ee</anchor>
      <arglist>(vec4D, x, y, z)</arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLOAT_MAX</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a2124ca619b2968b0cc8429e2a7036869</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLOAT_NAN</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a5d44c195a8722bab20bb0953a1114e3d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLOAT_INF</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>ac93cd804695f999a50f816740b18182d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>isfinite</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a256d70a32c03904ce13c999c1e9dc35d</anchor>
      <arglist>(x)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetMathBreakOnAssume</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a66b155c2afacb1202344006749b1d616</anchor>
      <arglist>(bool isEnabled)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>MathBreakOnAssume</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a7ae2d272b0387b15780bd65968c8a956</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>DegToRad</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>acfd5668fc781d74859693a79d086024d</anchor>
      <arglist>(const float3 &amp;degrees)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>DegToRad</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a06f07dbc9a347a218d730140560e8e87</anchor>
      <arglist>(float degrees)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>RadToDeg</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a8b727df85fa0fc5b1cdff9bddb59e6bd</anchor>
      <arglist>(const float3 &amp;radians)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>RadToDeg</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a90955dc7ffb04f2514cbf86bb64d87ea</anchor>
      <arglist>(float radians)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Cos</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a386e3fbef0409028b9236e949f18b7cd</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Sin</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a0c9ba19f74b19cd71f780c899a1a72da</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Sqrt</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a28313dfeda7c503b70ae96a4e3ab1bc3</anchor>
      <arglist>(float v)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Pow</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a31011e04d8be8d674c381cabe763b487</anchor>
      <arglist>(float base, float exp)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Factorial</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a0e7da4d7a69c5b5afb707807aa177017</anchor>
      <arglist>(int n)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CombinatorialRec</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a7dace71cdf9b9e7b3297a6cec5bc93ee</anchor>
      <arglist>(int n, int k)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>CombinatorialTab</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>ab04af431a63f29c0e20f5a67c15f25b6</anchor>
      <arglist>(int n, int k)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>PowInt</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a7d4f6ec790e5c466fcda5bc3aa53b970</anchor>
      <arglist>(float base, int exponent)</arglist>
    </member>
    <member kind="function">
      <type>T</type>
      <name>Clamp</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a980958b8f13bec4e9bea56689d924c6c</anchor>
      <arglist>(const T &amp;val, const T &amp;floor, const T &amp;ceil)</arglist>
    </member>
    <member kind="function">
      <type>T</type>
      <name>Clamp01</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>ab5cce216da433fa72b04ae5fd6a349a8</anchor>
      <arglist>(const T &amp;val)</arglist>
    </member>
    <member kind="function">
      <type>const T</type>
      <name>Min</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a6899d90f3d779571b4582d0c97082e39</anchor>
      <arglist>(const T &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const T</type>
      <name>Max</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>af0a80b63feb178f3d4b5eb51d8518de0</anchor>
      <arglist>(const T &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Swap</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a3354c94d2114b9544e68b5cb34cf2321</anchor>
      <arglist>(T &amp;a, T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>GreaterThan</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>ade53a63d30a112589c2fbd69b2ed9f9e</anchor>
      <arglist>(const T &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>LessThan</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a16faee82709b934a63e557c1bfe092c8</anchor>
      <arglist>(const T &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const T</type>
      <name>Abs</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a6021295637171749b6e8fb1de0bba2fd</anchor>
      <arglist>(const T &amp;a)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Equal</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a53287f5a6d6119a2a21604dabdd90c50</anchor>
      <arglist>(const T &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>EqualAbsT</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a925876ed2dfbfcf6c5a13348f05308df</anchor>
      <arglist>(const T &amp;a, const T &amp;b, const T &amp;epsilon)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>EqualAbs</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a11dbe8861e2959f7e0b90a7ab75dcf7b</anchor>
      <arglist>(float a, float b, float epsilon=1e-6f)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>EqualRelT</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a2b6c529c07457a78d37b59e9ca7d713d</anchor>
      <arglist>(const T &amp;a, const T &amp;b, const T &amp;maxRelError)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>EqualRel</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>aa597488fc5fcd5f6ec2d2677f26b235e</anchor>
      <arglist>(float a, float b, float maxRelError=1e-5f)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>EqualUlps</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a47f0b718fb5f39a0298cb283c6d83dcb</anchor>
      <arglist>(float a, float b, int maxUlps=10000)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFiniteNumber</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>aa1e5029bfb1f3e63d6ec81af9afadd46</anchor>
      <arglist>(const T &amp;value)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFiniteNumber&lt; float &gt;</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a662e9bf276342b00f946042cf316fc3f</anchor>
      <arglist>(const float &amp;value)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFiniteNumber&lt; double &gt;</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>a3ca388786eaffbb5235590b81062e69a</anchor>
      <arglist>(const double &amp;value)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFiniteNumber&lt; long double &gt;</name>
      <anchorfile>_math_func_8h.html</anchorfile>
      <anchor>ad3bef0aeb14a86fd01a48d1f9146fde8</anchor>
      <arglist>(const long double &amp;value)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>MathFwd.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_math_fwd_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>CONST_WIN32</name>
      <anchorfile>_math_fwd_8h.html</anchorfile>
      <anchor>a4d9e4a48aaf6992c6a22989751be38f1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>assert</name>
      <anchorfile>_math_fwd_8h.html</anchorfile>
      <anchor>af576bf8ffa22a44e53018c67095ffbf0</anchor>
      <arglist>(x)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>MathOps.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_math_ops_8cpp</filename>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <member kind="function">
      <type>bool</type>
      <name>EqualAbs</name>
      <anchorfile>_math_ops_8cpp.html</anchorfile>
      <anchor>a088d63ff623e10af46d0df67d4fab962</anchor>
      <arglist>(float a, float b, float epsilon)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>EqualRel</name>
      <anchorfile>_math_ops_8cpp.html</anchorfile>
      <anchor>ae7572e18da20af842c7af8eaea797a9f</anchor>
      <arglist>(float a, float b, float maxRelError)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>EqualUlps</name>
      <anchorfile>_math_ops_8cpp.html</anchorfile>
      <anchor>a10ccb7bd69558bc4709fa068c40da824</anchor>
      <arglist>(float a, float b, int maxUlps)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Matrix.inl</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_matrix_8inl</filename>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Quat.h</includes>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateX</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>ace70c17920218dfd46db8cc4a3da7865</anchor>
      <arglist>(Matrix &amp;m, float angle)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateY</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>adfb63232426c057447b037b2b443798c</anchor>
      <arglist>(Matrix &amp;m, float angle)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateZ</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a73169dc2fc413ab54703ee3554e516cf</anchor>
      <arglist>(Matrix &amp;m, float angle)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateEulerXYZ</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a7bf1e5a4e40281683e635debaceb0026</anchor>
      <arglist>(Matrix &amp;m, float x, float y, float z)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ExtractEulerXYZ</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>ab76b8024d031bb8631c0e91f9d212699</anchor>
      <arglist>(Matrix &amp;m, float &amp;x, float &amp;y, float &amp;z)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateEulerXZY</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a2936ca03528da7f501a57262ad0fa8e7</anchor>
      <arglist>(Matrix &amp;m, float &amp;x, float &amp;z, float &amp;y)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ExtractEulerXZY</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>ac5a8bd757546ef0a74a2c1435c661c46</anchor>
      <arglist>(Matrix &amp;m, float &amp;x, float &amp;z, float &amp;y)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateEulerYXZ</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a4b69cd577c46038eb9517099c1017e17</anchor>
      <arglist>(Matrix &amp;m, float &amp;y, float &amp;x, float &amp;z)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ExtractEulerYXZ</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a7a568e98135eb735c9ecf1a4f82a6056</anchor>
      <arglist>(Matrix &amp;m, float &amp;y, float &amp;x, float &amp;z)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateEulerYZX</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a24329ddbb17a2f3fe1dfa44afa2e404a</anchor>
      <arglist>(Matrix &amp;m, float &amp;y, float &amp;z, float &amp;x)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ExtractEulerYZX</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a9734fd1fc28f867e9184951b62840de8</anchor>
      <arglist>(Matrix &amp;m, float &amp;y, float &amp;z, float &amp;x)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateEulerZXY</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>ab4e264bc276e2be88bf6dbb7101e2ceb</anchor>
      <arglist>(Matrix &amp;m, float &amp;z, float &amp;x, float &amp;y)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ExtractEulerZXY</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>aa06e85ffccc9f056ddd1ef472e06c18f</anchor>
      <arglist>(Matrix &amp;m, float &amp;z, float &amp;x, float &amp;y)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateEulerZYX</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a6dfe121486b6946103051e1d00468b7c</anchor>
      <arglist>(Matrix &amp;m, float &amp;z, float &amp;y, float &amp;x)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ExtractEulerZYX</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>ad66d1543b09999d8f7e455d0f2819ab2</anchor>
      <arglist>(Matrix &amp;m, float &amp;z, float &amp;y, float &amp;x)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateEulerXYX</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>ab366b014d895fcc9eea7bb954c08d29f</anchor>
      <arglist>(Matrix &amp;m, float &amp;x2, float &amp;y, float &amp;x1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ExtractEulerXYX</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>acb8c4b2b05fd3b92fa4eafa2214d74ef</anchor>
      <arglist>(Matrix &amp;m, float &amp;x2, float &amp;y, float &amp;x1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateEulerXZX</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a8487fd753cc265d9bc702935324a77f2</anchor>
      <arglist>(Matrix &amp;m, float &amp;x2, float &amp;z, float &amp;x1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ExtractEulerXZX</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a9bcaf8aaa3b6c425ef38ffaafd85b16a</anchor>
      <arglist>(Matrix &amp;m, float &amp;x2, float &amp;z, float &amp;x1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateEulerYXY</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a6b5db02860b15f446dc6b51cd59fd39b</anchor>
      <arglist>(Matrix &amp;m, float &amp;y2, float &amp;x, float &amp;y1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ExtractEulerYXY</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>aa9d204d9c969e199de1573bfb2866a74</anchor>
      <arglist>(Matrix &amp;m, float &amp;y2, float &amp;x, float &amp;y1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateEulerYZY</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a52662edd996da7c44ce2f3a4c3a74863</anchor>
      <arglist>(Matrix &amp;m, float &amp;y2, float &amp;z, float &amp;y1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ExtractEulerYZY</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a4c9fe4c9029eb41cdbed5ee47866e07d</anchor>
      <arglist>(Matrix &amp;m, float &amp;y2, float &amp;z, float &amp;y1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateEulerZXZ</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>ac33f8dad1e4eea2ca21aa0ad65186977</anchor>
      <arglist>(Matrix &amp;m, float &amp;z2, float &amp;x, float &amp;z1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ExtractEulerZXZ</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>af30d46546543ae4084517fdf26203da2</anchor>
      <arglist>(Matrix &amp;m, float &amp;z2, float &amp;x, float &amp;z1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3PartRotateEulerZYZ</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>adf331fc5cc7f593289d39b967f745dfe</anchor>
      <arglist>(Matrix &amp;m, float &amp;z2, float &amp;y, float &amp;z1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ExtractEulerZYZ</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>ac2131e29909205f664f398edafc0ee0a</anchor>
      <arglist>(Matrix &amp;m, float &amp;z2, float &amp;y, float &amp;z1)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetIdentity3x3</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>aa28bdeb0707e081634d789b956e45f4d</anchor>
      <arglist>(Matrix &amp;m)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>InverseAffineMatrixNoScale</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>add9e1c49bfa1e651a48b2a349428666a</anchor>
      <arglist>(Matrix &amp;mat)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>InverseAffineMatrixUniformScale</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a10ad8f3ef20b2f5f61574160668ec96c</anchor>
      <arglist>(Matrix &amp;mat)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>InverseAffineMatrixNonuniformScale</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a5a1464a5445fb251929e466a09be57b0</anchor>
      <arglist>(Matrix &amp;mat)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>InverseMatrix</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a577db245b574c9bc35f57c2d4f6fa72e</anchor>
      <arglist>(Matrix &amp;mat)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>LUDecomposeMatrix</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a77c2204e5ef1def66edf04813eb31716</anchor>
      <arglist>(const Matrix &amp;mat, Matrix &amp;lower, Matrix &amp;upper)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>CholeskyDecomposeMatrix</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a6e6b8b2957650a2a8277c9901c20e2b7</anchor>
      <arglist>(const Matrix &amp;mat, Matrix &amp;lower)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetMatrixRotatePart</name>
      <anchorfile>_matrix_8inl.html</anchorfile>
      <anchor>a1e8dc0eb4eb330749e85b105d0afb21c</anchor>
      <arglist>(Matrix &amp;m, const Quat &amp;q)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>MatrixProxy.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_matrix_proxy_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <class kind="class">MatrixProxy</class>
  </compound>
  <compound kind="file">
    <name>OBB.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_o_b_b_8cpp</filename>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <includes id="_o_b_b_8h" name="OBB.h" local="yes" imported="no">OBB.h</includes>
    <includes id="_a_a_b_b_8h" name="AABB.h" local="yes" imported="no">AABB.h</includes>
    <includes id="_l_c_g_8h" name="LCG.h" local="yes" imported="no">LCG.h</includes>
    <includes id="_line_segment_8h" name="LineSegment.h" local="yes" imported="no">LineSegment.h</includes>
    <includes id="_line_8h" name="Line.h" local="yes" imported="no">Line.h</includes>
    <includes id="_plane_8h" name="Plane.h" local="yes" imported="no">Plane.h</includes>
    <includes id="_sphere_8h" name="Sphere.h" local="yes" imported="no">Sphere.h</includes>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4_8h" name="float4.h" local="yes" imported="no">float4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">float4x4.h</includes>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Quat.h</includes>
    <includes id="_ray_8h" name="Ray.h" local="yes" imported="no">Ray.h</includes>
    <includes id="_triangle_8h" name="Triangle.h" local="yes" imported="no">Triangle.h</includes>
    <member kind="function">
      <type>void</type>
      <name>OBBSetFrom</name>
      <anchorfile>_o_b_b_8cpp.html</anchorfile>
      <anchor>a1e15d3c76386dcc63a532561f1eeb7aa</anchor>
      <arglist>(OBB &amp;obb, const AABB &amp;aabb, const Matrix &amp;m)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>OBBTransform</name>
      <anchorfile>_o_b_b_8cpp.html</anchorfile>
      <anchor>a31c92333f16fa712481f68b08e6691f0</anchor>
      <arglist>(OBB &amp;o, const Matrix &amp;transform)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>OBB.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_o_b_b_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">OBB</class>
  </compound>
  <compound kind="file">
    <name>Plane.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_plane_8cpp</filename>
    <includes id="_a_a_b_b_8h" name="AABB.h" local="yes" imported="no">AABB.h</includes>
    <includes id="_circle_8h" name="Circle.h" local="yes" imported="no">Circle.h</includes>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <includes id="_plane_8h" name="Plane.h" local="yes" imported="no">Plane.h</includes>
    <includes id="_line_8h" name="Line.h" local="yes" imported="no">Line.h</includes>
    <includes id="_o_b_b_8h" name="OBB.h" local="yes" imported="no">OBB.h</includes>
    <includes id="_ray_8h" name="Ray.h" local="yes" imported="no">Ray.h</includes>
    <includes id="_sphere_8h" name="Sphere.h" local="yes" imported="no">Sphere.h</includes>
    <includes id="_triangle_8h" name="Triangle.h" local="yes" imported="no">Triangle.h</includes>
    <includes id="_line_segment_8h" name="LineSegment.h" local="yes" imported="no">LineSegment.h</includes>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4_8h" name="float4.h" local="yes" imported="no">float4.h</includes>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Quat.h</includes>
    <member kind="function">
      <type>bool</type>
      <name>IntersectLinePlane</name>
      <anchorfile>_plane_8cpp.html</anchorfile>
      <anchor>ab71d09a2a437c8014a210a153b4ad5cd</anchor>
      <arglist>(const float3 &amp;ptOnPlane, const float3 &amp;planeNormal, const float3 &amp;lineStart, const float3 &amp;lineDir, float *t)</arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>operator*</name>
      <anchorfile>_plane_8cpp.html</anchorfile>
      <anchor>a22556bb30a3bb2dbfe8962a9fc89f57a</anchor>
      <arglist>(const float3x3 &amp;transform, const Plane &amp;plane)</arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>operator*</name>
      <anchorfile>_plane_8cpp.html</anchorfile>
      <anchor>a2fcab5de138426494149507999605ccd</anchor>
      <arglist>(const float3x4 &amp;transform, const Plane &amp;plane)</arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>operator*</name>
      <anchorfile>_plane_8cpp.html</anchorfile>
      <anchor>afaee9dee725c537792f835e0be480d83</anchor>
      <arglist>(const float4x4 &amp;transform, const Plane &amp;plane)</arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>operator*</name>
      <anchorfile>_plane_8cpp.html</anchorfile>
      <anchor>a92bf82f126d6a6320da23bea8b18bf14</anchor>
      <arglist>(const Quat &amp;transform, const Plane &amp;plane)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Plane.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_plane_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">Plane</class>
    <member kind="function">
      <type>Plane</type>
      <name>operator*</name>
      <anchorfile>_plane_8h.html</anchorfile>
      <anchor>a22556bb30a3bb2dbfe8962a9fc89f57a</anchor>
      <arglist>(const float3x3 &amp;transform, const Plane &amp;plane)</arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>operator*</name>
      <anchorfile>_plane_8h.html</anchorfile>
      <anchor>a2fcab5de138426494149507999605ccd</anchor>
      <arglist>(const float3x4 &amp;transform, const Plane &amp;plane)</arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>operator*</name>
      <anchorfile>_plane_8h.html</anchorfile>
      <anchor>afaee9dee725c537792f835e0be480d83</anchor>
      <arglist>(const float4x4 &amp;transform, const Plane &amp;plane)</arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>operator*</name>
      <anchorfile>_plane_8h.html</anchorfile>
      <anchor>a92bf82f126d6a6320da23bea8b18bf14</anchor>
      <arglist>(const Quat &amp;transform, const Plane &amp;plane)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Polygon.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_polygon_8cpp</filename>
    <includes id="_polygon_8h" name="Polygon.h" local="yes" imported="no">Polygon.h</includes>
    <includes id="_plane_8h" name="Plane.h" local="yes" imported="no">Plane.h</includes>
    <includes id="_line_8h" name="Line.h" local="yes" imported="no">Line.h</includes>
    <includes id="_line_segment_8h" name="LineSegment.h" local="yes" imported="no">LineSegment.h</includes>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
  </compound>
  <compound kind="file">
    <name>Polygon.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_polygon_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">Polygon</class>
  </compound>
  <compound kind="file">
    <name>Polynomial.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_polynomial_8h</filename>
    <class kind="class">Polynomial</class>
  </compound>
  <compound kind="file">
    <name>Quat.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_quat_8cpp</filename>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Quat.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <includes id="float4_8h" name="float4.h" local="yes" imported="no">float4.h</includes>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">float4x4.h</includes>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <member kind="function">
      <type>Quat</type>
      <name>Lerp</name>
      <anchorfile>_quat_8cpp.html</anchorfile>
      <anchor>a3376f63d4c665d0fb8c22d3a35efb289</anchor>
      <arglist>(const Quat &amp;a, const Quat &amp;b, float t)</arglist>
    </member>
    <member kind="function">
      <type>Quat</type>
      <name>Slerp</name>
      <anchorfile>_quat_8cpp.html</anchorfile>
      <anchor>a1bb096ca6d18eb2b273aa9b3edda889e</anchor>
      <arglist>(const Quat &amp;a, const Quat &amp;b, float t)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetQuatFrom</name>
      <anchorfile>_quat_8cpp.html</anchorfile>
      <anchor>a75ec466642d54c57260e2002f2244891</anchor>
      <arglist>(Quat &amp;q, const M &amp;m)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Quat.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_quat_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <class kind="class">Quat</class>
    <member kind="function">
      <type>Quat</type>
      <name>Lerp</name>
      <anchorfile>_quat_8h.html</anchorfile>
      <anchor>a3376f63d4c665d0fb8c22d3a35efb289</anchor>
      <arglist>(const Quat &amp;a, const Quat &amp;b, float t)</arglist>
    </member>
    <member kind="function">
      <type>Quat</type>
      <name>Slerp</name>
      <anchorfile>_quat_8h.html</anchorfile>
      <anchor>a1bb096ca6d18eb2b273aa9b3edda889e</anchor>
      <arglist>(const Quat &amp;a, const Quat &amp;b, float t)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Ray.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_ray_8cpp</filename>
    <includes id="_a_a_b_b_8h" name="AABB.h" local="yes" imported="no">AABB.h</includes>
    <includes id="_line_8h" name="Line.h" local="yes" imported="no">Line.h</includes>
    <includes id="_ray_8h" name="Ray.h" local="yes" imported="no">Ray.h</includes>
    <includes id="_line_segment_8h" name="LineSegment.h" local="yes" imported="no">LineSegment.h</includes>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">float4x4.h</includes>
    <includes id="_o_b_b_8h" name="OBB.h" local="yes" imported="no">OBB.h</includes>
    <includes id="_plane_8h" name="Plane.h" local="yes" imported="no">Plane.h</includes>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Quat.h</includes>
    <includes id="_sphere_8h" name="Sphere.h" local="yes" imported="no">Sphere.h</includes>
    <includes id="_triangle_8h" name="Triangle.h" local="yes" imported="no">Triangle.h</includes>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <member kind="function">
      <type>Ray</type>
      <name>operator*</name>
      <anchorfile>_ray_8cpp.html</anchorfile>
      <anchor>ab9763649eebd92cadd9b1a727eee4b2b</anchor>
      <arglist>(const float3x3 &amp;transform, const Ray &amp;ray)</arglist>
    </member>
    <member kind="function">
      <type>Ray</type>
      <name>operator*</name>
      <anchorfile>_ray_8cpp.html</anchorfile>
      <anchor>af13304396e4ac823e76eda6893da9252</anchor>
      <arglist>(const float3x4 &amp;transform, const Ray &amp;ray)</arglist>
    </member>
    <member kind="function">
      <type>Ray</type>
      <name>operator*</name>
      <anchorfile>_ray_8cpp.html</anchorfile>
      <anchor>ad6d87ac185a5851ae0de860157f5010d</anchor>
      <arglist>(const float4x4 &amp;transform, const Ray &amp;ray)</arglist>
    </member>
    <member kind="function">
      <type>Ray</type>
      <name>operator*</name>
      <anchorfile>_ray_8cpp.html</anchorfile>
      <anchor>ab3e20905e080aeafddbf6de6b78a9bd0</anchor>
      <arglist>(const Quat &amp;transform, const Ray &amp;ray)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Ray.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_ray_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">Ray</class>
    <member kind="function">
      <type>Ray</type>
      <name>operator*</name>
      <anchorfile>_ray_8h.html</anchorfile>
      <anchor>ab9763649eebd92cadd9b1a727eee4b2b</anchor>
      <arglist>(const float3x3 &amp;transform, const Ray &amp;ray)</arglist>
    </member>
    <member kind="function">
      <type>Ray</type>
      <name>operator*</name>
      <anchorfile>_ray_8h.html</anchorfile>
      <anchor>af13304396e4ac823e76eda6893da9252</anchor>
      <arglist>(const float3x4 &amp;transform, const Ray &amp;ray)</arglist>
    </member>
    <member kind="function">
      <type>Ray</type>
      <name>operator*</name>
      <anchorfile>_ray_8h.html</anchorfile>
      <anchor>ad6d87ac185a5851ae0de860157f5010d</anchor>
      <arglist>(const float4x4 &amp;transform, const Ray &amp;ray)</arglist>
    </member>
    <member kind="function">
      <type>Ray</type>
      <name>operator*</name>
      <anchorfile>_ray_8h.html</anchorfile>
      <anchor>ab3e20905e080aeafddbf6de6b78a9bd0</anchor>
      <arglist>(const Quat &amp;transform, const Ray &amp;ray)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Rect.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_rect_8h</filename>
    <class kind="class">Rect</class>
  </compound>
  <compound kind="file">
    <name>Sphere.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_sphere_8cpp</filename>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <includes id="_o_b_b_8h" name="OBB.h" local="yes" imported="no">OBB.h</includes>
    <includes id="_a_a_b_b_8h" name="AABB.h" local="yes" imported="no">AABB.h</includes>
    <includes id="_l_c_g_8h" name="LCG.h" local="yes" imported="no">LCG.h</includes>
    <includes id="_line_segment_8h" name="LineSegment.h" local="yes" imported="no">LineSegment.h</includes>
    <includes id="_line_8h" name="Line.h" local="yes" imported="no">Line.h</includes>
    <includes id="_ray_8h" name="Ray.h" local="yes" imported="no">Ray.h</includes>
    <includes id="_plane_8h" name="Plane.h" local="yes" imported="no">Plane.h</includes>
    <includes id="_sphere_8h" name="Sphere.h" local="yes" imported="no">Sphere.h</includes>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4_8h" name="float4.h" local="yes" imported="no">float4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">float4x4.h</includes>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Quat.h</includes>
    <includes id="_triangle_8h" name="Triangle.h" local="yes" imported="no">Triangle.h</includes>
    <member kind="function">
      <type>bool</type>
      <name>IntersectLineSphere</name>
      <anchorfile>_sphere_8cpp.html</anchorfile>
      <anchor>adfcf75f0cf0e55c1660202ec5347558a</anchor>
      <arglist>(const float3 &amp;lPos, const float3 &amp;lDir, const Sphere &amp;s, float &amp;t)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Sphere.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_sphere_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">Sphere</class>
  </compound>
  <compound kind="file">
    <name>TransformOps.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_transform_ops_8cpp</filename>
    <includes id="_transform_ops_8h" name="TransformOps.h" local="yes" imported="no">TransformOps.h</includes>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <includes id="float4_8h" name="float4.h" local="yes" imported="no">float4.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">float4x4.h</includes>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8cpp.html</anchorfile>
      <anchor>ad7cedd946cd864505ad699f49c81bd6d</anchor>
      <arglist>(const TranslateOp &amp;lhs, const float3x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8cpp.html</anchorfile>
      <anchor>afab119c89552543cb11e8d0f3f133f99</anchor>
      <arglist>(const float3x4 &amp;lhs, const TranslateOp &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8cpp.html</anchorfile>
      <anchor>ab05712274a07154d153682efdfec690d</anchor>
      <arglist>(const TranslateOp &amp;lhs, const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8cpp.html</anchorfile>
      <anchor>ab6dbec6a630806376d4e0682a86f6c4c</anchor>
      <arglist>(const float4x4 &amp;lhs, const TranslateOp &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8cpp.html</anchorfile>
      <anchor>a06a506584b553a822dfb539f8fbdcdc1</anchor>
      <arglist>(const ScaleOp &amp;lhs, const float3x3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8cpp.html</anchorfile>
      <anchor>a4b3f1a9c6db37096077613e9c05c17f2</anchor>
      <arglist>(const float3x3 &amp;lhs, const ScaleOp &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8cpp.html</anchorfile>
      <anchor>a0c7c9184820418e0de1db83d53d5f9bf</anchor>
      <arglist>(const ScaleOp &amp;lhs, const float3x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8cpp.html</anchorfile>
      <anchor>acaa859dd3ac15d17cfa483272a727896</anchor>
      <arglist>(const float3x4 &amp;lhs, const ScaleOp &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8cpp.html</anchorfile>
      <anchor>a8450a4f3a01d53256eb5b6a1930afa4e</anchor>
      <arglist>(const ScaleOp &amp;lhs, const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8cpp.html</anchorfile>
      <anchor>ae1604055f3d4b54285f2c6c31f4ba0da</anchor>
      <arglist>(const float4x4 &amp;lhs, const ScaleOp &amp;rhs)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>TransformOps.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_transform_ops_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">TranslateOp</class>
    <class kind="class">ScaleOp</class>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8h.html</anchorfile>
      <anchor>ad7cedd946cd864505ad699f49c81bd6d</anchor>
      <arglist>(const TranslateOp &amp;lhs, const float3x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8h.html</anchorfile>
      <anchor>afab119c89552543cb11e8d0f3f133f99</anchor>
      <arglist>(const float3x4 &amp;lhs, const TranslateOp &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8h.html</anchorfile>
      <anchor>ab05712274a07154d153682efdfec690d</anchor>
      <arglist>(const TranslateOp &amp;lhs, const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8h.html</anchorfile>
      <anchor>ab6dbec6a630806376d4e0682a86f6c4c</anchor>
      <arglist>(const float4x4 &amp;lhs, const TranslateOp &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8h.html</anchorfile>
      <anchor>a06a506584b553a822dfb539f8fbdcdc1</anchor>
      <arglist>(const ScaleOp &amp;lhs, const float3x3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8h.html</anchorfile>
      <anchor>a4b3f1a9c6db37096077613e9c05c17f2</anchor>
      <arglist>(const float3x3 &amp;lhs, const ScaleOp &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8h.html</anchorfile>
      <anchor>a0c7c9184820418e0de1db83d53d5f9bf</anchor>
      <arglist>(const ScaleOp &amp;lhs, const float3x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8h.html</anchorfile>
      <anchor>acaa859dd3ac15d17cfa483272a727896</anchor>
      <arglist>(const float3x4 &amp;lhs, const ScaleOp &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8h.html</anchorfile>
      <anchor>a8450a4f3a01d53256eb5b6a1930afa4e</anchor>
      <arglist>(const ScaleOp &amp;lhs, const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>_transform_ops_8h.html</anchorfile>
      <anchor>ae1604055f3d4b54285f2c6c31f4ba0da</anchor>
      <arglist>(const float4x4 &amp;lhs, const ScaleOp &amp;rhs)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Triangle.cpp</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_triangle_8cpp</filename>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">MathFunc.h</includes>
    <includes id="float2_8h" name="float2.h" local="yes" imported="no">float2.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <includes id="float3x3_8h" name="float3x3.h" local="yes" imported="no">float3x3.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">float3x4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">float4x4.h</includes>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Quat.h</includes>
    <includes id="_triangle_8h" name="Triangle.h" local="yes" imported="no">Triangle.h</includes>
    <includes id="_plane_8h" name="Plane.h" local="yes" imported="no">Plane.h</includes>
    <includes id="_line_8h" name="Line.h" local="yes" imported="no">Line.h</includes>
    <includes id="_line_segment_8h" name="LineSegment.h" local="yes" imported="no">LineSegment.h</includes>
    <includes id="_ray_8h" name="Ray.h" local="yes" imported="no">Ray.h</includes>
    <includes id="_sphere_8h" name="Sphere.h" local="yes" imported="no">Sphere.h</includes>
    <includes id="_a_a_b_b_8h" name="AABB.h" local="yes" imported="no">AABB.h</includes>
    <includes id="_o_b_b_8h" name="OBB.h" local="yes" imported="no">OBB.h</includes>
    <member kind="function">
      <type>bool</type>
      <name>IntersectLineTri</name>
      <anchorfile>_triangle_8cpp.html</anchorfile>
      <anchor>a72b061a6c36be253da80d51d98ca1935</anchor>
      <arglist>(const float3 &amp;linePos, const float3 &amp;lineDir, const float3 &amp;v0, const float3 &amp;v1, const float3 &amp;v2, float &amp;u, float &amp;v, float &amp;t)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>FindIntersectingLineSegments</name>
      <anchorfile>_triangle_8cpp.html</anchorfile>
      <anchor>ada6ea58d31cf8a38d08188647a99af0d</anchor>
      <arglist>(const Triangle &amp;t, float da, float db, float dc, LineSegment &amp;l1, LineSegment &amp;l2)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>RangesOverlap</name>
      <anchorfile>_triangle_8cpp.html</anchorfile>
      <anchor>a4d10ee7b03604db361b6b97018271f90</anchor>
      <arglist>(float start1, float end1, float start2, float end2)</arglist>
    </member>
    <member kind="function">
      <type>Triangle</type>
      <name>operator*</name>
      <anchorfile>_triangle_8cpp.html</anchorfile>
      <anchor>a8da4f178514a7810d5916e1a3b586719</anchor>
      <arglist>(const float3x3 &amp;transform, const Triangle &amp;t)</arglist>
    </member>
    <member kind="function">
      <type>Triangle</type>
      <name>operator*</name>
      <anchorfile>_triangle_8cpp.html</anchorfile>
      <anchor>abcc7f8b48e5db8461aaf45fd9ef29a79</anchor>
      <arglist>(const float3x4 &amp;transform, const Triangle &amp;t)</arglist>
    </member>
    <member kind="function">
      <type>Triangle</type>
      <name>operator*</name>
      <anchorfile>_triangle_8cpp.html</anchorfile>
      <anchor>a0b73a3d1122bab5bbfd0d307d87e813a</anchor>
      <arglist>(const float4x4 &amp;transform, const Triangle &amp;t)</arglist>
    </member>
    <member kind="function">
      <type>Triangle</type>
      <name>operator*</name>
      <anchorfile>_triangle_8cpp.html</anchorfile>
      <anchor>a5a0cb6153926240a3a5c39188b20a26d</anchor>
      <arglist>(const Quat &amp;transform, const Triangle &amp;t)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Triangle.h</name>
    <path>E:/Projects/TundraLCNew/Framework/Math/</path>
    <filename>_triangle_8h</filename>
    <includes id="_math_fwd_8h" name="MathFwd.h" local="yes" imported="no">MathFwd.h</includes>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">float3.h</includes>
    <class kind="class">Triangle</class>
    <member kind="function">
      <type>Triangle</type>
      <name>operator*</name>
      <anchorfile>_triangle_8h.html</anchorfile>
      <anchor>a8da4f178514a7810d5916e1a3b586719</anchor>
      <arglist>(const float3x3 &amp;transform, const Triangle &amp;t)</arglist>
    </member>
    <member kind="function">
      <type>Triangle</type>
      <name>operator*</name>
      <anchorfile>_triangle_8h.html</anchorfile>
      <anchor>abcc7f8b48e5db8461aaf45fd9ef29a79</anchor>
      <arglist>(const float3x4 &amp;transform, const Triangle &amp;t)</arglist>
    </member>
    <member kind="function">
      <type>Triangle</type>
      <name>operator*</name>
      <anchorfile>_triangle_8h.html</anchorfile>
      <anchor>a0b73a3d1122bab5bbfd0d307d87e813a</anchor>
      <arglist>(const float4x4 &amp;transform, const Triangle &amp;t)</arglist>
    </member>
    <member kind="function">
      <type>Triangle</type>
      <name>operator*</name>
      <anchorfile>_triangle_8h.html</anchorfile>
      <anchor>a5a0cb6153926240a3a5c39188b20a26d</anchor>
      <arglist>(const Quat &amp;transform, const Triangle &amp;t)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Transform.h</name>
    <path>E:/Projects/TundraLCNew/Framework/</path>
    <filename>_transform_8h</filename>
    <includes id="float3_8h" name="float3.h" local="yes" imported="no">Math/float3.h</includes>
    <includes id="float4_8h" name="float4.h" local="yes" imported="no">Math/float4.h</includes>
    <includes id="float3x4_8h" name="float3x4.h" local="yes" imported="no">Math/float3x4.h</includes>
    <includes id="float4x4_8h" name="float4x4.h" local="yes" imported="no">Math/float4x4.h</includes>
    <includes id="_transform_ops_8h" name="TransformOps.h" local="yes" imported="no">Math/TransformOps.h</includes>
    <includes id="_quat_8h" name="Quat.h" local="yes" imported="no">Math/Quat.h</includes>
    <includes id="_math_func_8h" name="MathFunc.h" local="yes" imported="no">Math/MathFunc.h</includes>
    <class kind="class">Transform</class>
  </compound>
  <compound kind="class">
    <name>AABB</name>
    <filename>class_a_a_b_b.html</filename>
    <member kind="function">
      <type></type>
      <name>AABB</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a5f5baf6c533905aa1456b3a3eb57bab2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>AABB</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a63c8b9b5d0bd60b789ea272d0acab3c4</anchor>
      <arglist>(const float3 &amp;minPoint, const float3 &amp;maxPoint)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>AABB</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a0a4859ebfc8d67f3464da0857a9f0e20</anchor>
      <arglist>(const OBB &amp;obb)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>AABB</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a822c45c0623344bd53c088ec332f5597</anchor>
      <arglist>(const Sphere &amp;s)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetNegativeInfinity</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a0d4bfded40c554173695f7664831ac71</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetCenter</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>af0b81a44931e9f9d0ec6eaa39322f2bc</anchor>
      <arglist>(const float3 &amp;center, const float3 &amp;halfSize)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFrom</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>ac8e2ad915ddca7f43e7a3d8873f6a8f2</anchor>
      <arglist>(const OBB &amp;obb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFrom</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a416d3f5d258a28685c3f517ebc704a7d</anchor>
      <arglist>(const Sphere &amp;s)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFrom</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a82b6afb1df146dbe770661d213dd7db0</anchor>
      <arglist>(const float3 *pointArray, int numPoints)</arglist>
    </member>
    <member kind="function">
      <type>OBB</type>
      <name>ToOBB</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>ab99b6fbd3d5a298930db55ba4f9ce67b</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Sphere</type>
      <name>MinimalEnclosingSphere</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a836432081d347c3c255e7004d478741d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Sphere</type>
      <name>MaximalContainedSphere</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a8c4e6a1778d9c374f53b392de01ea46d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFinite</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>ab588d203ea38c0d3986aa80dfa5a4c0a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsDegenerate</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a0649e3b4b6c2990f35e9f42ae18df576</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>CenterPoint</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a6b37f3c7c8ba8d777584b721529959b4</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>PointInside</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a09f897e0f254db385ec696b0cdf040da</anchor>
      <arglist>(float x, float y, float z) const </arglist>
    </member>
    <member kind="function">
      <type>LineSegment</type>
      <name>Edge</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>afd7c921ad2ac2d4d3b6410994f7168b3</anchor>
      <arglist>(int edgeIndex) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>CornerPoint</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>aeb955228e07312fa1a876c0be3daea4d</anchor>
      <arglist>(int cornerIndex) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>PointOnEdge</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a50f0f1f5c3073e2a940b1f9d10d37c5a</anchor>
      <arglist>(int edgeIndex, float u) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>FaceCenterPoint</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>ae9ca5c23293393fd44cf4e778c850819</anchor>
      <arglist>(int faceIndex) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>FacePoint</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a6e43bed4196255e23593ac65d2dab531</anchor>
      <arglist>(int faceIndex, float u, float v) const </arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>FacePlane</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a5ce5fd2eb134c88acf803e7a16da888e</anchor>
      <arglist>(int faceIndex) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>GetCornerPoints</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>abaf0f74eb1ff91301437c37e831d525d</anchor>
      <arglist>(float3 *outPointArray) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>GetFacePlanes</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>ab2ac1f16fe9b1fc630af3d4931638cce</anchor>
      <arglist>(Plane *outPlaneArray) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Size</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>ab52b349f4a9d38a6cb32fc396126a018</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>HalfSize</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>ac73011c769237dd79798e5f6bcc173ba</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Diagonal</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a5e7033a3b2af54717692a7938e4b18a1</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>HalfDiagonal</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a574924031bb7ad64f0ea9c8add3d4501</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Volume</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a97dc5d286137d3ecdf9e49d63b0f37ba</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>SurfaceArea</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a05b50f66773a9bbe18e0cb897954c7ac</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>RandomPointInside</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>ad9671ea9bf91af08bf30e11502e8363f</anchor>
      <arglist>(LCG &amp;rng) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>RandomPointOnSurface</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a8383759d666ca9bae1c801fc3d2e9268</anchor>
      <arglist>(LCG &amp;rng) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>RandomPointOnEdge</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a29df86839092c0181bdeb2d2940d6ab4</anchor>
      <arglist>(LCG &amp;rng) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>RandomCornerPoint</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a2f1f064b726e44ace99acf8b959bc0a9</anchor>
      <arglist>(LCG &amp;rng) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Translate</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a375a0f03ed5bc788fc848e4ceff184d2</anchor>
      <arglist>(const float3 &amp;offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Scale</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>aa88ebc691d4abd1e4d0399649c606b1a</anchor>
      <arglist>(const float3 &amp;centerPoint, float scaleFactor)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Scale</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>ab7f835a70c96d4f650764ebbad3b0abb</anchor>
      <arglist>(const float3 &amp;centerPoint, const float3 &amp;scaleFactor)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>TransformAsAABB</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a4c2c0dbfc63789e60c173ab9e905c201</anchor>
      <arglist>(const float3x3 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>TransformAsAABB</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>aefe37c93264e135bbd5a146a6507f382</anchor>
      <arglist>(const float3x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>TransformAsAABB</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>ad10eda61151359769fb5c4f497fdf147</anchor>
      <arglist>(const float4x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>TransformAsAABB</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a69521cc4d0050dd89ae05c822fc4e6ae</anchor>
      <arglist>(const Quat &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>OBB</type>
      <name>Transform</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a91df81faa5320fb15262572a0a0b1df6</anchor>
      <arglist>(const float3x3 &amp;transform) const </arglist>
    </member>
    <member kind="function">
      <type>OBB</type>
      <name>Transform</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a7d16b3a56a92e51cb818249c242e7d3b</anchor>
      <arglist>(const float3x4 &amp;transform) const </arglist>
    </member>
    <member kind="function">
      <type>OBB</type>
      <name>Transform</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>af1feb1211f7183d5546365d879b71063</anchor>
      <arglist>(const float4x4 &amp;transform) const </arglist>
    </member>
    <member kind="function">
      <type>OBB</type>
      <name>Transform</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>abc72bac50d0ee509986b4e33316b5a3f</anchor>
      <arglist>(const Quat &amp;transform) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>ad46023c503f947dc30c27bbf9882e69b</anchor>
      <arglist>(const float3 &amp;targetPoint) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>af5e9b69e98f94863a16264f585cd5fa9</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a3499e195b83eb40e56d69d2b5dab3135</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>ab360bbb49893116a86c36fd94e3d6e3c</anchor>
      <arglist>(const LineSegment &amp;lineSegment) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a24349e56215dda7faca9cd92a218461b</anchor>
      <arglist>(const AABB &amp;aabb) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>adb29d6d2f0f8ce9262a98732d19712e9</anchor>
      <arglist>(const Ray &amp;ray, float *dNear, float *dFar) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a51dde03e6ec70b07ef685cfdd8fd9e45</anchor>
      <arglist>(const Line &amp;line, float *dNear, float *dFar) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>ab9d8bf21b0c527851538513069cece55</anchor>
      <arglist>(const LineSegment &amp;lineSegment, float *dNear, float *dFar) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a35c00e874eb5836bdf836c6a4bffd928</anchor>
      <arglist>(const Plane &amp;plane) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a6c420ade812932be95a03b73802ef101</anchor>
      <arglist>(const OBB &amp;obb) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a20f1f8c5ba050b35fdd09e55eacff42f</anchor>
      <arglist>(const Sphere &amp;sphere, float3 *closestPointOnAABB) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a7059bff5e82514236f08b7c73d55e61a</anchor>
      <arglist>(const Triangle &amp;triangle) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProjectToAxis</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>aed74616b30c876ab69af71a5a8a82e25</anchor>
      <arglist>(const float3 &amp;axis, float &amp;dMin, float &amp;dMax) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Enclose</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a86c31c44947f6d1ed68c3c6520723b10</anchor>
      <arglist>(const float3 &amp;point)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Enclose</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a9b11ae811796ceaf647bd0a77933c8a1</anchor>
      <arglist>(const LineSegment &amp;lineSegment)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Enclose</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a7db432ac9ce79c07c5b32ae5a0da759d</anchor>
      <arglist>(const AABB &amp;aabb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Enclose</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>aa5324bee7e088b9accbbb0e930ed42e9</anchor>
      <arglist>(const OBB &amp;obb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Enclose</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a6bb0b46bd9e94b9f7c68c94a08cc99c1</anchor>
      <arglist>(const Sphere &amp;sphere)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Enclose</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a24a7e3dd754abeb47143e821c94d3742</anchor>
      <arglist>(const float3 *pointArray, int numPoints)</arglist>
    </member>
    <member kind="function">
      <type>AABB</type>
      <name>Intersection</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>acc6a1e39214efd3c478f6c1749a03205</anchor>
      <arglist>(const AABB &amp;aabb) const </arglist>
    </member>
    <member kind="function" static="yes">
      <type>static AABB</type>
      <name>MinimalEnclosingAABB</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>af9fb7abc87d68fd52c586d10c894af28</anchor>
      <arglist>(const float3 *pointArray, int numPoints)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>ExtremePointsAlongAABB</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a8cb707ff5df76dbdb64137a511c1f45f</anchor>
      <arglist>(const float3 *pointArray, int numPoints, int &amp;minx, int &amp;maxx, int &amp;miny, int &amp;maxy, int &amp;minz, int &amp;maxz)</arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>minPoint</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>a985df52d816938465cdb389631d0931a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>maxPoint</name>
      <anchorfile>class_a_a_b_b.html</anchorfile>
      <anchor>ae5bc483ab64d371554322a5a6e50da6f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>BitMaskT</name>
    <filename>class_bit_mask_t.html</filename>
    <templarg>Pos</templarg>
    <templarg>Bits</templarg>
    <member kind="enumvalue">
      <name>val</name>
      <anchorfile>class_bit_mask_t.html</anchorfile>
      <anchor>a1311502b86e59914982fed0680c33ef4a637d7b93d40d309018002a0b3f8de543</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Circle</name>
    <filename>class_circle.html</filename>
    <member kind="function">
      <type></type>
      <name>Circle</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>ad1ecfcfc7bf34529c6a6d6c448bf70fe</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Circle</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>a599b8ebb5a840e5d7511e18e0ec34a7f</anchor>
      <arglist>(const float3 &amp;center, const float3 &amp;normal, float radius)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>BasisU</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>a26416e1a7faf81cd9aaf343d9b208cd9</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>BasisV</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>a014e0ee3a4dcf0c20054048ee527ec08</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>GetPoint</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>a15b0f3855ee73ce1f23c224c957be48a</anchor>
      <arglist>(float angleRadians) const </arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>ContainingPlane</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>a3655c028863c56112df5e1832660b9ee</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>EdgeContains</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>abdc862d931201d83f2de28e0f5a4a109</anchor>
      <arglist>(const float3 &amp;point, float maxDistance=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>DiscContains</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>ac5c6d7373889cd284e31e3ed1c33b1d9</anchor>
      <arglist>(const float3 &amp;point, float maxDistance=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>DistanceToEdge</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>a1161896b6c9edc85a27223d2bf58d818</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>DistanceToEdge</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>ad394eb34c0bb2ce217e1889a5c2c5a6a</anchor>
      <arglist>(const Ray &amp;ray, float *d, float3 *closestPoint) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>DistanceToEdge</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>a0d7bc06732748279d8f93ed25e6f4ad2</anchor>
      <arglist>(const LineSegment &amp;lineSegment, float *d, float3 *closestPoint) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>DistanceToEdge</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>af641e74523cee803d5643655c6e8fa3d</anchor>
      <arglist>(const Line &amp;line, float *d, float3 *closestPoint) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPointToEdge</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>ae76f859f5ccbae0b2ceee3bd5c4bfca0</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPointToEdge</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>a0d21b61e75f3fac88cff1fc2f3dc47fd</anchor>
      <arglist>(const Ray &amp;ray, float *d) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPointToEdge</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>a2d94105f6263d05e7d80173542ccf1f0</anchor>
      <arglist>(const LineSegment &amp;lineSegment, float *d) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPointToEdge</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>a3c5e6a88e72c786292221a06962ca0b8</anchor>
      <arglist>(const Line &amp;line, float *d) const </arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>pos</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>a95f808076bb2ba91377a7ff68490c73f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>normal</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>a1a3b3c1fdb0c61e7b4d00811e22bcdb3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>r</name>
      <anchorfile>class_circle.html</anchorfile>
      <anchor>a7bdbdaf7361494f294d9a18626d7500f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>CombinatorialT</name>
    <filename>class_combinatorial_t.html</filename>
    <templarg>N</templarg>
    <templarg>K</templarg>
    <member kind="enumvalue">
      <name>val</name>
      <anchorfile>class_combinatorial_t.html</anchorfile>
      <anchor>a645fd45842ff014a3b98c5056adc2ce2a95b77257230015250c8befaef0d6b4ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>N</name>
      <anchorfile>class_combinatorial_t.html</anchorfile>
      <anchor>a645fd45842ff014a3b98c5056adc2ce2a32dd91120bface702ef23e03933de395</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>val</name>
      <anchorfile>class_combinatorial_t.html</anchorfile>
      <anchor>a645fd45842ff014a3b98c5056adc2ce2a95b77257230015250c8befaef0d6b4ee</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Complex</name>
    <filename>class_complex.html</filename>
    <member kind="function">
      <type></type>
      <name>Complex</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a43b9f07cdf697c71b5fd506a6cc80b8f</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Complex</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>ab917c6d4e99e2503c54bb00790f11892</anchor>
      <arglist>(float real, float imaginary)</arglist>
    </member>
    <member kind="function">
      <type>Complex</type>
      <name>Conjugate</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>aa4213153bdd369fbac1dfd99dd1c8e2c</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Normalize</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a8e65fcb87005c022a000b8792fe2331c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>Complex</type>
      <name>Normalized</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a4de7ae0bf98b3609bd2fcfe690bf8274</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Length</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>aa18f291fe26ff00dcaae146ed52c0117</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>LengthSq</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a51e55f34bdd03fc84293ed52f3135528</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Complex</type>
      <name>operator+</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a81044da20cb3143ce84895ebcbea3156</anchor>
      <arglist>(float real) const </arglist>
    </member>
    <member kind="function">
      <type>Complex</type>
      <name>operator+</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a85335af4619e9e47aa9c0846d1d149b0</anchor>
      <arglist>(const Complex &amp;c) const </arglist>
    </member>
    <member kind="function">
      <type>Complex</type>
      <name>operator-</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a559b3d4120b0d37f60de47c11c594838</anchor>
      <arglist>(float real) const </arglist>
    </member>
    <member kind="function">
      <type>Complex</type>
      <name>operator-</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a9db6e1faaf7164c809d3567085b5c89f</anchor>
      <arglist>(const Complex &amp;c) const </arglist>
    </member>
    <member kind="function">
      <type>Complex</type>
      <name>operator*</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>aad4a05398e755ad5fe50b0dde0410c62</anchor>
      <arglist>(float real) const </arglist>
    </member>
    <member kind="function">
      <type>Complex</type>
      <name>operator*</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a86f4398baa4ea8b9b52c39ad66f0a763</anchor>
      <arglist>(const Complex &amp;c) const </arglist>
    </member>
    <member kind="function">
      <type>Complex</type>
      <name>operator/</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>ab36580014a554590f4ffe9af53aa7156</anchor>
      <arglist>(float real) const </arglist>
    </member>
    <member kind="function">
      <type>Complex</type>
      <name>operator/</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a8a4d2bf57c76d1eed742b6385ffce365</anchor>
      <arglist>(const Complex &amp;c) const </arglist>
    </member>
    <member kind="function">
      <type>Complex &amp;</type>
      <name>operator+=</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a0607a12864f610eb443a59d9d7c891f5</anchor>
      <arglist>(float real)</arglist>
    </member>
    <member kind="function">
      <type>Complex &amp;</type>
      <name>operator+=</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>aaf6afc785a9cfe56d2cf0661a6badf9c</anchor>
      <arglist>(const Complex &amp;c)</arglist>
    </member>
    <member kind="function">
      <type>Complex &amp;</type>
      <name>operator-=</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a1d48014d816db8be8b59eef27124d4a1</anchor>
      <arglist>(float real)</arglist>
    </member>
    <member kind="function">
      <type>Complex &amp;</type>
      <name>operator-=</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>af1339cb859e35752f440584e13bfcb2c</anchor>
      <arglist>(const Complex &amp;c)</arglist>
    </member>
    <member kind="function">
      <type>Complex &amp;</type>
      <name>operator*=</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a27936371af53f3728deaa5565efed797</anchor>
      <arglist>(float real)</arglist>
    </member>
    <member kind="function">
      <type>Complex &amp;</type>
      <name>operator*=</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>af14ba5c214174e3a2333dbc84c5ae5bd</anchor>
      <arglist>(const Complex &amp;c)</arglist>
    </member>
    <member kind="function">
      <type>Complex &amp;</type>
      <name>operator/=</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a9838c4bce6f9e3ce462465b2d3a49859</anchor>
      <arglist>(float real)</arglist>
    </member>
    <member kind="function">
      <type>Complex &amp;</type>
      <name>operator/=</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a3fc725a2f0507d8925f0194a53324e74</anchor>
      <arglist>(const Complex &amp;c)</arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>r</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a66a574ca3e39bd1476a7fc1670980052</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>i</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>ad64f4aa1d5e37061c106f6d739a2dd7b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const Complex</type>
      <name>zero</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>a95aa85cdf91d2b184e19036ba6c5fa15</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const Complex</type>
      <name>unitOne</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>aeffc0ae6e1817c00dc1ec97d8ecb2a09</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const Complex</type>
      <name>unitI</name>
      <anchorfile>class_complex.html</anchorfile>
      <anchor>af9214bb50251fc14483d59c4e948ce37</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Cone</name>
    <filename>class_cone.html</filename>
    <member kind="function">
      <type></type>
      <name>Cone</name>
      <anchorfile>class_cone.html</anchorfile>
      <anchor>aec709e915b3271a750d420b14b215bfb</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_cone.html</anchorfile>
      <anchor>a73b17955b059aa0d62519f021c06b7d9</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Cylinder</name>
    <filename>class_cylinder.html</filename>
    <member kind="function">
      <type></type>
      <name>Cylinder</name>
      <anchorfile>class_cylinder.html</anchorfile>
      <anchor>a01dc978cb576f834b9545e43d4dad2a2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_cylinder.html</anchorfile>
      <anchor>a6374f82d7c59766fa8082fb8266bd7fa</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Ellipsoid</name>
    <filename>class_ellipsoid.html</filename>
    <member kind="function">
      <type></type>
      <name>Ellipsoid</name>
      <anchorfile>class_ellipsoid.html</anchorfile>
      <anchor>a8aba92dbd635c581c3a5e96f285dd651</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_ellipsoid.html</anchorfile>
      <anchor>aa231ae2450cf855fdcf35fd0f7873c43</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>FactorialT</name>
    <filename>class_factorial_t.html</filename>
    <templarg>N</templarg>
    <member kind="enumvalue">
      <name>val</name>
      <anchorfile>class_factorial_t.html</anchorfile>
      <anchor>a8e3d0348f7a1e1f853a88bef7291241ea493409985b8c647a8aad6f3c3fb05081</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>float2</name>
    <filename>classfloat2.html</filename>
    <member kind="enumvalue">
      <name>Size</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>aba22dd67f5271857ad08b42f25ca5e85abbfda5062f3cb60478cb620a9b0dc9c9</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float2</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a491ca6caabfa5f7eae5734817dac7774</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float2</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a8c0d90a91473a5f45d51f2fea088fd70</anchor>
      <arglist>(const float2 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float2</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>acf2d8df02f83affbd68e2bd6b8010b2b</anchor>
      <arglist>(float x, float y)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float2</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a0cb08ad03f73ebacdee4cdf93d7c63a1</anchor>
      <arglist>(const float *data)</arglist>
    </member>
    <member kind="function">
      <type>float *</type>
      <name>ptr</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>af3e631bf7690942064453a42cb3c5de6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const float *</type>
      <name>ptr</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a648b32598dcdfb315641fc8cb306d712</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float</type>
      <name>operator[]</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>acf9a21191a3b01518f9eb3aed3581d1f</anchor>
      <arglist>(int index) const </arglist>
    </member>
    <member kind="function">
      <type>float &amp;</type>
      <name>operator[]</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a19e4469e2f2a0c3c55dacd1f222adddb</anchor>
      <arglist>(int index)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>LengthSq</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a7eccada9a7d0caaa6989865ad5066c12</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Length</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>afcabf15cd80fb343eeb860b439af2d41</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Normalize</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a6ce6019e80f98e9b6db03e72bf8072e8</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Normalized</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a79a965365e7c59889451ef6973fde013</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>ScaleToLength</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a076421f5de5cbd0e6f2f290a13e1c06c</anchor>
      <arglist>(float newLength)</arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>ScaledToLength</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>aa30aa34e591569c084091e8109883617</anchor>
      <arglist>(float newLength) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsNormalized</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a360da68a7cde689261baec6f05a4efef</anchor>
      <arglist>(float epsilonSq=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsZero</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>aed2bad4a2133f974e058b72c5888056c</anchor>
      <arglist>(float epsilonSq=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFinite</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>ac638e4eb1815b96b8b28f11ec47301e1</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsPerpendicular</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>ae82b467bb4281335cf2f6df034bc4c9d</anchor>
      <arglist>(const float2 &amp;other, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Equals</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>ae27a22f9e8b204e0c4003f878de5b741</anchor>
      <arglist>(const float2 &amp;rhs, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Equals</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a67d1eb998e1924ff795dca5eec0fa5fd</anchor>
      <arglist>(float x, float y, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>SumOfElements</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a1570178d20ee60cc35aa8cf43a74aea3</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>ProductOfElements</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a7f2602af492575109992e8ef144036f0</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>AverageOfElements</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>af485f25c493b44835ba865c52dc96416</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>MinElement</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a01f125f81c26232d20294074cc5bf783</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>MinElementIndex</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a4d4b17eefb86a9dcb5a86fab8f934c4f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>MaxElement</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>ac56ee2e784035131242bee4a59095db5</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>MaxElementIndex</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a9ec37e80c0eae11947f0c7c25999553c</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Abs</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a401c2329c794c95c7ab720f65630c1bd</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Min</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a36c63b58bdb80820e81fc5585a557018</anchor>
      <arglist>(float ceil) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Min</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a0cb2dd3665da87290b34b54c3b407fa2</anchor>
      <arglist>(const float2 &amp;ceil) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Max</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>af01ee9608cf5338be4d2ffdfa696ff5e</anchor>
      <arglist>(float floor) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Max</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>ac5992f5cdf00aaa1a2798d865ac71502</anchor>
      <arglist>(const float2 &amp;floor) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Clamp</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>ae175be868c6bb59bb7c0bf7ee6b58b6d</anchor>
      <arglist>(float floor, float ceil) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Clamp</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>aa76ff110ee1a2037cf5e4123adeef8d9</anchor>
      <arglist>(const float2 &amp;floor, const float2 &amp;ceil) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Clamp01</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a3712d41a7fea195820adf5bef354e922</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a144f0058b30c7f3ea43ee946eee423da</anchor>
      <arglist>(const float2 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>DistanceSq</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>abf0aba842ae6cd78dc7c4848ae3249e7</anchor>
      <arglist>(const float2 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Dot</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a108c05368a0cf8e438e098ba6d41f408</anchor>
      <arglist>(const float2 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>PerpDot</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a8650ee452bf5791e359acc34a722f2ef</anchor>
      <arglist>(const float2 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Reflect</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>afd0d3922a0d3727fd2ddf7df90f23a77</anchor>
      <arglist>(const float2 &amp;normal) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Refract</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a040bb2ed7e96e146fae992ddbe09dea9</anchor>
      <arglist>(const float2 &amp;normal, float negativeSideRefractionIndex, float positiveSideRefractionIndex) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>ProjectTo</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a0d6db740bcc72db45804035d858957fc</anchor>
      <arglist>(const float2 &amp;direction) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>ProjectToNorm</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a6c1067b077313b955a9b90097100381f</anchor>
      <arglist>(const float2 &amp;direction) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>AngleBetween</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a9e4bff08479a6b9b343b572dd3bace3b</anchor>
      <arglist>(const float2 &amp;other) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>AngleBetweenNorm</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a71bd2bdc216312bae809be80718c94e6</anchor>
      <arglist>(const float2 &amp;normalizedVector) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Decompose</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>aa1752b6d5383ecbd335dc81553bb3b18</anchor>
      <arglist>(const float2 &amp;direction, float2 &amp;outParallel, float2 &amp;outPerpendicular) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Lerp</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a199f0d886dd7845af665f2067fe45fcb</anchor>
      <arglist>(const float2 &amp;b, float t) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFromScalar</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a5fc0e5a8f719a0c985de38c7b6b2f841</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>ae4c691ce84446ab8d2ee0e8384394947</anchor>
      <arglist>(float x, float y)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Rotate90CW</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>abdce5d286fb2f3f87c4afe75ee485e47</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Rotated90CW</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a177435cb27eb657a9e4703f1e2d65ccf</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Rotate90CCW</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>ac9ceec51456a4108a87a31029fd34c83</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Rotated90CCW</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a6b322ca68f652878faf2a7fdc4de7b7d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>operator-</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a5064c1b5d017dd1fa882ded2fbad90d7</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>operator+</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a9ec2a52ac4316f2ca3b899c74619636f</anchor>
      <arglist>(const float2 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>operator-</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a2775930433e6054e1f8b724ade47804d</anchor>
      <arglist>(const float2 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>operator*</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a3bc52f4a2a3f85ce79ac232461381bff</anchor>
      <arglist>(float scalar) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>operator/</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a34f3bc48c26723ab49189972291d37b7</anchor>
      <arglist>(float scalar) const </arglist>
    </member>
    <member kind="function">
      <type>float2 &amp;</type>
      <name>operator+=</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a57d326ca45fe492a9aaaa093e88efc81</anchor>
      <arglist>(const float2 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float2 &amp;</type>
      <name>operator-=</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>ad298d3c87ab5558a005d47a89cf3b75a</anchor>
      <arglist>(const float2 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float2 &amp;</type>
      <name>operator*=</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a95ec71428f06214a59ec5725a7a13fbb</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>float2 &amp;</type>
      <name>operator/=</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a7e0de24cd70bbff6201c5ca2df297a6c</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Add</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a151dcb1c19b29a700cff5acd650fed6b</anchor>
      <arglist>(const float2 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Sub</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a65353dc7820dc9745a499189f23fc3bf</anchor>
      <arglist>(const float2 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Mul</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a42543159c2b09c65f28e1cd9d4dc9f01</anchor>
      <arglist>(float rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Div</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a53ed8abdccedfdef92f5dd1e47fd1c8b</anchor>
      <arglist>(float rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Neg</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a403720175f67529263fd5d88d6413965</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>Mul</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a14f4c5357f51a3e924dabaecec820883</anchor>
      <arglist>(const float2 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float2</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>aec4d2ec43f2152b2abcd64aed84623ce</anchor>
      <arglist>(const QVector2D &amp;other)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator QVector2D</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a24d4f6b761a02c74c8f517b9501c6a53</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator QString</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a2b7b880bfb48ef50e451bf8999dbb78e</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QString</type>
      <name>toString</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a3be55003c068bae64b717785f9e4b131</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QVector2D</type>
      <name>ToQVector2D</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a185babf53066aed21013906f48ff0f7a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float2</type>
      <name>FromString</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a2965d7fb06bc3b00f0e50671ee93ed57</anchor>
      <arglist>(const char *str)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float2</type>
      <name>Lerp</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a7493d167fbd7c833d82b319c6bbed28c</anchor>
      <arglist>(const float2 &amp;a, const float2 &amp;b, float t)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Orthogonalize</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>ab9bea4b3d8546b3fd600b97f36d051d4</anchor>
      <arglist>(const float2 &amp;a, float2 &amp;b)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Orthonormalize</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a4121ba950c7b538c865f88e98614fc7b</anchor>
      <arglist>(float2 &amp;a, float2 &amp;b)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float2</type>
      <name>FromScalar</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a8a1d992926e25fa2e26e0127f861001a</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static bool</type>
      <name>OrientedCCW</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a5093852b87d16cf3bbe2b0153d7fbaed</anchor>
      <arglist>(const float2 &amp;a, const float2 &amp;b, const float2 &amp;c)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float</type>
      <name>MinAreaRect</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a7dba5a1df16b9eef8ded09c03858b4ec</anchor>
      <arglist>(const float2 *pointArray, int numPoints, float2 &amp;center, float2 &amp;uDir, float2 &amp;vDir)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float2</type>
      <name>FromQVector2D</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a9651b69db8f4f97e8318cf94be0c02e0</anchor>
      <arglist>(const QVector2D &amp;v)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float2</type>
      <name>FromString</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>add617df5ed4d80f27ef9792a1f35480f</anchor>
      <arglist>(const QString &amp;str)</arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>x</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a0bb68b4a8de04ffea5f7aae53c48a613</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>y</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a0f1298f22c4ee20a369cd3b9c25b2cc6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float2</type>
      <name>zero</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a4fd4f62a14b0a87d209b3f57cd97fe73</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float2</type>
      <name>one</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>ac4fca28bd27640ffb8cb6769ac01ba58</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float2</type>
      <name>unitX</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a0237af5f7a48ffe951d839c337de4528</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float2</type>
      <name>unitY</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>a8d8001d2154e3ab9be066e4cfd81a3f0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float2</type>
      <name>nan</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>ad54063f247aacd629dde21e2240a7a13</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float2</type>
      <name>inf</name>
      <anchorfile>classfloat2.html</anchorfile>
      <anchor>afaf623d0a9c4cf0c86b5b2ff01687c6b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>float3</name>
    <filename>classfloat3.html</filename>
    <member kind="enumvalue">
      <name>Size</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>af51ce03c3b812ae248913c07a6ea28c3ab86c96dd82d4f32a13b65047c54ce6d8</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>adf12f33c77fddf168c255d4ac6f93b07</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>ac8262352458aa4b3d0dc9e5afa31c312</anchor>
      <arglist>(const float3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a285d1970e16dd09d85a10deb8ce83fa7</anchor>
      <arglist>(float x, float y, float z)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a37bf36294fc3c3d49313bfb41da028ca</anchor>
      <arglist>(const float2 &amp;xy, float z)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>afde07f1814f3ef001ebcbb838641e6dc</anchor>
      <arglist>(const float *data)</arglist>
    </member>
    <member kind="function">
      <type>float *</type>
      <name>ptr</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a4f165c2b9bc0e020263aa37230d421a4</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const float *</type>
      <name>ptr</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a9b19a7c2ea5442e6c8d0e8b0c03b9053</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float</type>
      <name>operator[]</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a679a8afd4957c31ccb199e5764701d22</anchor>
      <arglist>(int index) const </arglist>
    </member>
    <member kind="function">
      <type>float &amp;</type>
      <name>operator[]</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>ab058a6382e3a3743bae2848e28221b7f</anchor>
      <arglist>(int index)</arglist>
    </member>
    <member kind="function">
      <type>float2</type>
      <name>xy</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a3b40fc94e82b8e6bbb8cacc3c9e70bda</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>LengthSq</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a10ac192fb68ed6e251852b4fffa09a68</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Length</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a316461c437a708047c0505f067cd993e</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Normalize</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a7f69a7a79c38c9f53c486222220088ed</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Normalized</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a8c8b5a528c81ec74b2e997b44e512bd8</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>ScaleToLength</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a3f0831d362138fa9998b8fb4aa1735c6</anchor>
      <arglist>(float newLength)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ScaledToLength</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a6cab8563e5ab05aec838c14f87ffe88b</anchor>
      <arglist>(float newLength) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsNormalized</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a567fc30da2923b17e1c936b09e0f419e</anchor>
      <arglist>(float epsilonSq=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsZero</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a03e7edf87c90c62ee8f35fc0c0481458</anchor>
      <arglist>(float epsilonSq=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFinite</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>ab2f503537b0151db4bf7f0d2c0d364e0</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsPerpendicular</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a9dbd6d46d87354485923d728d8aecea6</anchor>
      <arglist>(const float3 &amp;other, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Equals</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a359c71f1b9d5a2ebddab42453f691687</anchor>
      <arglist>(const float3 &amp;rhs, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Equals</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>adbe9baa797c74ba5132b2c7b6c9afd9d</anchor>
      <arglist>(float x, float y, float z, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>SumOfElements</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>aeca4fac014a2e11396f439d17f61f479</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>ProductOfElements</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a8d109325c2fe824ad26721728b3870f6</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>AverageOfElements</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a44e60f03a2dd9dd4b0621864533fda4a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>MinElement</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>aba40f286b1affc09abe23aca0498a201</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>MinElementIndex</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a0bef3b9c4f4932d6e10a398408dd404c</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>MaxElement</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a55d8e17adee5dd3bf51956d91d0e152a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>MaxElementIndex</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>ac2b445e41db30e06c2aa1ac9117dc23d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Abs</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a83beda406934eea085b505371fb42401</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Min</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>ae133bf88d88d160e514edc55f3c15f74</anchor>
      <arglist>(float ceil) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Min</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a995786f36160e764ce812bdb0a0415ca</anchor>
      <arglist>(const float3 &amp;ceil) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Max</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a5ebc9f4518a790e105efb4348fa1f5e9</anchor>
      <arglist>(float floor) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Max</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a67cc0ba5f5e5ca2752029719b5cb6ed4</anchor>
      <arglist>(const float3 &amp;floor) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Clamp</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>aec8925f21185b44f4c63a0dcaa898d0a</anchor>
      <arglist>(const float3 &amp;floor, const float3 &amp;ceil) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Clamp01</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>aea83dbfafd9b7cb2bb8e78e53bf88a27</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Clamp</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>ac03dc43349311a4650466b85efd25132</anchor>
      <arglist>(float floor, float ceil) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>ab72461a5b065b5e94203a59bd20b2e68</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>DistanceSq</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a00a3b62fabbd90a9bcdc53c2b87417b7</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Dot</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a3c8a943de7f23ac6f9a841f12e108f13</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Cross</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a6afb99bd3462fc9823363dee85000714</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>OuterProduct</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a139fb6ad5644df6b5981fc9b19cca8b5</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Perpendicular</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>afbd921f5166d11aac784a21196427815</anchor>
      <arglist>(const float3 &amp;hint=float3(0, 1, 0), const float3 &amp;hint2=float3(0, 0, 1)) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>AnotherPerpendicular</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>ac8f2c1a61fcd66bca5eb008f971ff9f7</anchor>
      <arglist>(const float3 &amp;hint=float3(0, 1, 0), const float3 &amp;hint2=float3(0, 0, 1)) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Reflect</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>ab8de7f5747ddc3cc2c7ab4877bed4eab</anchor>
      <arglist>(const float3 &amp;normal) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Refract</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a0f36d9c86fdf51d56f7f5ae96a5fd8c1</anchor>
      <arglist>(const float3 &amp;normal, float negativeSideRefractionIndex, float positiveSideRefractionIndex) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ProjectTo</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a697140b96c2042b66e9465f53d7d9e44</anchor>
      <arglist>(const float3 &amp;direction) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ProjectToNorm</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>acbcb432a348db47c8fdc730828ea125c</anchor>
      <arglist>(const float3 &amp;direction) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>AngleBetween</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a23522cab711817e2dc4f4f626fbb3ac5</anchor>
      <arglist>(const float3 &amp;other) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>AngleBetweenNorm</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a16d23a8080a30f30fafda73dec37827d</anchor>
      <arglist>(const float3 &amp;normalizedVector) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Decompose</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a497211487794eaccf3d6ef8f823dffed</anchor>
      <arglist>(const float3 &amp;direction, float3 &amp;outParallel, float3 &amp;outPerpendicular) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Lerp</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a6b2aa3eaa2bb9db8f332706a583c4102</anchor>
      <arglist>(const float3 &amp;b, float t) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFromScalar</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a614562526fa695804f5d42d113a748b8</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a08b15c5faa15ad05b26ba311259905d5</anchor>
      <arglist>(float x, float y, float z)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>ToPos4</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a87ffc45dbcc4d2d93e9d083e7e22300d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>ToDir4</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a604f25a5286d7801696be368efcfa6a4</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>operator-</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>af75ae7181d7bcc39ad30991d97ab0547</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>operator+</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a32e7a45c62e965afdd8113b86b6ca6bc</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>operator-</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a33dea92154eaac580ac4703b3c6c96ac</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>operator*</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a6bd1029c7f4cce4c69f156b6c664626b</anchor>
      <arglist>(float scalar) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>operator/</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a998059f80cfa34a33345f094e5ae962d</anchor>
      <arglist>(float scalar) const </arglist>
    </member>
    <member kind="function">
      <type>float3 &amp;</type>
      <name>operator+=</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a2bc0529c9e530c64ff65f1af4af90765</anchor>
      <arglist>(const float3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3 &amp;</type>
      <name>operator-=</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a3a1bd8124972b768eb3a5c8fbd734146</anchor>
      <arglist>(const float3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3 &amp;</type>
      <name>operator*=</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a632ac0df03634d0e58b51c9ca8108a51</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>float3 &amp;</type>
      <name>operator/=</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a4719d82df76dc4bf80531ca895c0b603</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Add</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>af49d7a53a2945a6e45394a01d1ad1091</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Sub</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>aea0c8edab6b83dc3e8e41566e3753ef2</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Mul</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>ac10237bf294ce7449d3b6e664b0f570e</anchor>
      <arglist>(float rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Div</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a3b054a78e3d2887bf9eef317455f4cf3</anchor>
      <arglist>(float rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Neg</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a56a3f70f461c7c02e85bec84cdca3a47</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Mul</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a075a7510b0a2bab9f65ab0f0af48a947</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>abe97e3d8b09339948ff6df6644727da7</anchor>
      <arglist>(const QVector3D &amp;other)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator QVector3D</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>acce8b0b0b8b654ee2ea6af3cc14e97de</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator QString</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a64ca423ed0a5d833f94e4bd221a38640</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QString</type>
      <name>toString</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a622dd1272bf1dbcb9975279d04d6cd64</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QVector3D</type>
      <name>ToQVector3D</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a46f0a03a8c0c180976b3c6c01ba17358</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3</type>
      <name>FromString</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a6ab63849de98751820497e165219b3d3</anchor>
      <arglist>(const char *str)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float</type>
      <name>ScalarTripleProduct</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a2fe7296dcaf783cdee3e6039939a0025</anchor>
      <arglist>(const float3 &amp;u, const float3 &amp;v, const float3 &amp;w)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3</type>
      <name>Lerp</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a82e2fdfc3139747077835a3a781fd6d8</anchor>
      <arglist>(const float3 &amp;a, const float3 &amp;b, float t)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Orthogonalize</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a99a0c5b25f4f9e59898f51f19dee18bd</anchor>
      <arglist>(const float3 &amp;a, float3 &amp;b)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Orthogonalize</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a6e418396422e8445e93314b7af8f85bb</anchor>
      <arglist>(const float3 &amp;a, float3 &amp;b, float3 &amp;c)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static bool</type>
      <name>AreOrthogonal</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a683d04569a48bb6dac5efff35159ba6e</anchor>
      <arglist>(const float3 &amp;a, const float3 &amp;b, float epsilon=1e-3f)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static bool</type>
      <name>AreOrthogonal</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>aa4770b7e5962ffbf0c6fa67572696815</anchor>
      <arglist>(const float3 &amp;a, const float3 &amp;b, const float3 &amp;c, float epsilon=1e-3f)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Orthonormalize</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>af04326aa44189694c7de516498e26ead</anchor>
      <arglist>(float3 &amp;a, float3 &amp;b)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Orthonormalize</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a637c6e9f4302b86df23d336933d334b1</anchor>
      <arglist>(float3 &amp;a, float3 &amp;b, float3 &amp;c)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static bool</type>
      <name>AreOrthonormal</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a41e07d3fb357db2b33a0b72860c940bc</anchor>
      <arglist>(const float3 &amp;a, const float3 &amp;b, float epsilon=1e-3f)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static bool</type>
      <name>AreOrthonormal</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>aefdbb5f745b77e405626971fc5508a55</anchor>
      <arglist>(const float3 &amp;a, const float3 &amp;b, const float3 &amp;c, float epsilon=1e-3f)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3</type>
      <name>FromScalar</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a2737ed71e4bc3e07d0860c4ec931fbe5</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3</type>
      <name>FromQVector3D</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a5421f78edfcda659b70ef495d5044f40</anchor>
      <arglist>(const QVector3D &amp;v)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3</type>
      <name>FromString</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>adf354b97c1b933a42aaf8b9475dbe99a</anchor>
      <arglist>(const QString &amp;str)</arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>x</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>af621f02abb1c788738fe61ea9807ff9c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>y</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>aa6147d421a81889971f8c66aa92abf0d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>z</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a772dffd42d89f350c5a1b766c4703245</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float3</type>
      <name>zero</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a8fa89b8703195369a62c0c67d2e48e55</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float3</type>
      <name>one</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>af15b2d484ac5ff583dca24aa3f17a6e5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float3</type>
      <name>unitX</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a7eba676a2dd0aa7292a2cce40ad65165</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float3</type>
      <name>unitY</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a0d77a09da35111a2a421aa3ce0301650</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float3</type>
      <name>unitZ</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a98995b3864f72862fdfbcc24568fb420</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float3</type>
      <name>nan</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a3c0027611e9bfb79b22346747ffc299d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float3</type>
      <name>inf</name>
      <anchorfile>classfloat3.html</anchorfile>
      <anchor>a2b181fe8f8598307c5c46ccc2f698ffc</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>float3x3</name>
    <filename>classfloat3x3.html</filename>
    <member kind="enumvalue">
      <name>Rows</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a06d53080382f7579b0aa63299b3a4284a612fa48187cd8b6d17efcd32320625bf</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>Cols</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a8fff9fa9eeed6101c9eaa5f6f0b9506da1eefc4d03ccfea1507cdc79be4957e37</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3x3</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a43cf4b3e2e02f524330c4159e8e8020f</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3x3</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ad4cd4cd6b3bb450dda135beff5850bfb</anchor>
      <arglist>(const float3x3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3x3</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ad86aee95862778835375b2ab705e1690</anchor>
      <arglist>(float _00, float _01, float _02, float _10, float _11, float _12, float _20, float _21, float _22)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3x3</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a869dc06544ff6f63897e7711d34d96d3</anchor>
      <arglist>(const float3 &amp;col0, const float3 &amp;col1, const float3 &amp;col2)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3x3</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a2f77d44c8276b1efb88f371fbb6eeb9e</anchor>
      <arglist>(const Quat &amp;orientation)</arglist>
    </member>
    <member kind="function">
      <type>Quat</type>
      <name>ToQuat</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a0fd5372a947f57de08d2cfddd372f0f8</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>GetScale</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a3bf40072836b2c21991f530d34eabc60</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>MatrixProxy&lt; Cols &gt; &amp;</type>
      <name>operator[]</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a53fe90b248b0f1cba2e3c7455958e87e</anchor>
      <arglist>(int row)</arglist>
    </member>
    <member kind="function">
      <type>const MatrixProxy&lt; Cols &gt; &amp;</type>
      <name>operator[]</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>abfbe6cdd9b9229707c416aeb4d91320a</anchor>
      <arglist>(int row) const </arglist>
    </member>
    <member kind="function">
      <type>float &amp;</type>
      <name>At</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a3fd88e65f18707955930783e9eb120c4</anchor>
      <arglist>(int row, int col)</arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float</type>
      <name>At</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ab7a75521520e02aade2ab34ef97aa9f4</anchor>
      <arglist>(int row, int col) const </arglist>
    </member>
    <member kind="function">
      <type>float3 &amp;</type>
      <name>Row</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a0c537d61242ceebe97941dca027777e0</anchor>
      <arglist>(int row)</arglist>
    </member>
    <member kind="function">
      <type>const float3 &amp;</type>
      <name>Row</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>affb09b83d56d198ea89608a99b295bc2</anchor>
      <arglist>(int row) const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float3</type>
      <name>Col</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ae0b58c7ab1d38bcbc29cee2784248da2</anchor>
      <arglist>(int col) const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float3</type>
      <name>Diagonal</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a2620719dbbc4d21ed09b133c919d5ebd</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ScaleRow</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a6a0260f72350444065da058dc1a5d0b9</anchor>
      <arglist>(int row, float scalar)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ScaleCol</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>af6a42274c3ac0bfd53f12411579ffb76</anchor>
      <arglist>(int col, float scalar)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>WorldX</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a6e28aed8539c520167bc80ecacab19ea</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>WorldY</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>afbdae2082fc80ecd8402af2748603d37</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>WorldZ</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>af66e8d79fc05b4068ff51a8b94a675b9</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float *</type>
      <name>ptr</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a55dc7a0462ae477c5fab45c4340db004</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const float *</type>
      <name>ptr</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>adedaa71a791d3438c0d6b4e643e701e1</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRow</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a1bdb7877cdb0471b04ae04d8dd6b8963</anchor>
      <arglist>(int row, float x, float y, float z)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRow</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a497ee6be794b96403b80cc36d9310dd1</anchor>
      <arglist>(int row, const float3 &amp;rowVector)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRow</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a77482c68824c8adfab1c7026e06894e0</anchor>
      <arglist>(int row, const float *data)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetCol</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a0d4ae063801874b46b89368bd3767150</anchor>
      <arglist>(int column, float x, float y, float z)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetCol</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>adbd8cc2ec012c0615cbb6c53b2cce19f</anchor>
      <arglist>(int column, const float3 &amp;columnVector)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetCol</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a3bb2563f1e81b3f9e7c2c861075d8207</anchor>
      <arglist>(int column, const float *data)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a1eb4121b281d1e21e56d837456574e57</anchor>
      <arglist>(float _00, float _01, float _02, float _10, float _11, float _12, float _20, float _21, float _22)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a393dae79eb1263e614b4bf2181d23ac0</anchor>
      <arglist>(const float3x3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a5908a0dd07eec92d669a41a0a28563f7</anchor>
      <arglist>(const float *values)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ac9e0939a495290bd0835ad3b19a8aa40</anchor>
      <arglist>(int row, int col, float value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetIdentity</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a395931472188bc3bfa6e1ca64316cd80</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SwapColumns</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a500773a46c61d51908c2a6e94dccce4d</anchor>
      <arglist>(int col1, int col2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SwapRows</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a0bb2c078df751b680e21481076f7e763</anchor>
      <arglist>(int row1, int row2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePartX</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a6852c340564b828a9046f0f8f98066c8</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePartY</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>afb20a1fa752e9b347ecd950e65106f67</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePartZ</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a838679d94a3a5b6f2b5cf3b89ccb1368</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePart</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a888dabcaf71012daebcc8585d89689bd</anchor>
      <arglist>(const float3 &amp;axisDirection, float angleRadians)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePart</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a26ef484ff782a25d570b410a6c0cf29c</anchor>
      <arglist>(const Quat &amp;orientation)</arglist>
    </member>
    <member kind="function">
      <type>float3x3 &amp;</type>
      <name>operator=</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>abc78bce1336238420be084dc914b43c5</anchor>
      <arglist>(const Quat &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x3 &amp;</type>
      <name>operator=</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a929b1d1cfd69195c6ea07e3900acdf39</anchor>
      <arglist>(const float3x3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Determinant</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a94ebf2674a5c0b05c4cb4e2be6c9f0b2</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Inverse</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ac74bc04339f437518b5fca817000b7a3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>Inverted</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a5a3784ed8dcb67340cbf7dd0debcd24f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>InverseOrthogonal</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a7cf2fbcff045005c5cce577a3175d131</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>InverseOrthogonalUniformScale</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a1ab27ad5e15f9ca81f35fda54d8a685e</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>InverseOrthonormal</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a72b88a46eea751aed01ca26ea28e2685</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transpose</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a34b6df49d18fb30ca9d32c78445df94b</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>Transposed</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a6ed51e364e5247906370bf38bcb07bd5</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>InverseTranspose</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a1ec99bf39169e0f7e03e31610d9f4f2c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>InverseTransposed</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ada95c9b45d4bf3c7c966546699fde4e3</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Trace</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a6dc705fc749efc23658af0d829fd1f00</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Orthonormalize</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ae05a1cb5f01e0ff18cb7b49d3114dd43</anchor>
      <arglist>(int firstColumn, int secondColumn, int thirdColumn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RemoveScale</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a308ca5f82cf76d9a59018b4da0115e7a</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Transform</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ae4dad6f136e95ede32f2dda42e2f9c02</anchor>
      <arglist>(const float3 &amp;vector) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Transform</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>afd055cfc6c89f0cb6da191e244acec8a</anchor>
      <arglist>(float x, float y, float z) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>TransformLeft</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a5111917891343c62bd70ffc286ae15f2</anchor>
      <arglist>(const float3 &amp;lhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Transform</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a2a1f45d7319b215386e5b192227a8416</anchor>
      <arglist>(const float4 &amp;vector) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>BatchTransform</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a8e8b3669a6f10b09da19129dc47e1c56</anchor>
      <arglist>(float3 *pointArray, int numPoints) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>BatchTransform</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a61347f7e146bd19fb311fdc3a6227cac</anchor>
      <arglist>(float3 *pointArray, int numPoints, int stride) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>BatchTransform</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a3beef824a499e02e1896926e593b40b3</anchor>
      <arglist>(float4 *vectorArray, int numVectors) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>BatchTransform</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>abd52bfc17864364e7865d6efc8604e63</anchor>
      <arglist>(float4 *vectorArray, int numVectors, int stride) const </arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>operator*</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>abacca277289ed228b20884bc3bbc3297</anchor>
      <arglist>(const float3x3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>operator*</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ae03e9d16781ad21f5339c928c151e5e3</anchor>
      <arglist>(const Quat &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>operator*</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a1bc5a055194358e4f46d57ed5021c181</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>operator*</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a6933ccb52938bb06cf88d54a7113d644</anchor>
      <arglist>(const float4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>operator*</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ac6a9357ac58142e6284bc555816297dd</anchor>
      <arglist>(float scalar) const </arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>operator/</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a50c180db22463595eb5a7e831e7605d2</anchor>
      <arglist>(float scalar) const </arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>operator+</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a56aa7ca043320e2cd37284546070156d</anchor>
      <arglist>(const float3x3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>operator-</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>aca655c6f01bf8d76e39da217a4c0e68c</anchor>
      <arglist>(const float3x3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>operator-</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a0eb58a61a8929692cbf6046b4e84b56d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3x3 &amp;</type>
      <name>operator*=</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>aba40bfdd8b85f27889eeff38e03be8ec</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>float3x3 &amp;</type>
      <name>operator/=</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a32ef4cdc52b7e5728410de686506d60e</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>float3x3 &amp;</type>
      <name>operator+=</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ab6b9a6afc171f3d0f2a9b1688e54378a</anchor>
      <arglist>(const float3x3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x3 &amp;</type>
      <name>operator-=</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a4f9ccafb2446e0115b850dfc33c5a597</anchor>
      <arglist>(const float3x3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFinite</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ac94f5114dfebd033583c6c866c6d4e24</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsIdentity</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a6a7ad0eea63ac6ca50d28703193e65dd</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsLowerTriangular</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a7665a5e085596b79153143a405ed30c8</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsUpperTriangular</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a42d2da2da8146a8d6f92de92ce70dbfc</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsInvertible</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>adced6bd1e498ee1b48f38cd856e63e12</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsSymmetric</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>af1aea7c5cab4ab4fe596cde577b37f0b</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsSkewSymmetric</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ae752d53ff07146d204587df51d7a890a</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>HasUnitaryScale</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a7b8a0753f812b600c8d2981bee306db9</anchor>
      <arglist>(float epsilonSq=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>HasNegativeScale</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ab8373581f75cc9e300a0149e3692735e</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>HasUniformScale</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ae84ed07d39d035fbe9682c1500bc01e5</anchor>
      <arglist>(float epsilonSq=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsOrthogonal</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a32cd3d6024a21fa7f03a98b17b4c780d</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsOrthonormal</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a8cf4e10d3c8281bc10d405d9fa84f5ef</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Equals</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>afd41a07c9502188117dde2c4050c4546</anchor>
      <arglist>(const float3x3 &amp;other, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXYX</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ab2f8142a2daa1fcb5602a84c02283658</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXZX</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a7a4011673a33a08549dc6fefea0ff306</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYXY</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>af55028d1387dc1c402975dc11fe9a468</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYZY</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ad10e3ee6c7d7a7651cf752fc6b5d4e1d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZXZ</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a3b2b2667d96d174b057703c7eb2db0c0</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZYZ</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a9a87daed396feae66ef11d069bbef52e</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXYZ</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>aa5a39f99ce77c746b249f595e381bb62</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXZY</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ac92f56b0d0bac38ef9f353342bb01010</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYXZ</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a79e79b0a14328dd23082c75bf5daffca</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYZX</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a72bcc4ac0c03e07d14248a49fef3730f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZXY</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>abb55160c0b7d85d92574ef2578bf32e3</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZYX</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a647f77a4d5e7a40bfcad7b3eb5e12ed0</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ExtractScale</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>acec7fa9798f84bc6a3e6df04701406eb</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Decompose</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a7198c0b6ec9b7f556b5aa18acd519137</anchor>
      <arglist>(Quat &amp;rotate, float3 &amp;scale) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Decompose</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>affd4581f2e545041d22b26c6818a3b6d</anchor>
      <arglist>(float3x3 &amp;rotate, float3 &amp;scale) const </arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>Mul</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a23ee15c235ddeea93480a39c4030f44c</anchor>
      <arglist>(const float3x3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>Mul</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a523d7b9242c34a655d0b71847abd8734</anchor>
      <arglist>(const float3x4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>Mul</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>aba9132c89a6ecc8ed0e030a99876ef99</anchor>
      <arglist>(const float4x4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>Mul</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a63e2f4226b24e3192a9bc8d487ea0b22</anchor>
      <arglist>(const Quat &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Mul</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a029d96ddf113fdf255c5e933674060e3</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>MulPos</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a727011fdd408b13bca86ed0845e615cb</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>MulDir</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a367924a013507134a527ff5e9d39902b</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator QString</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a69413df94f6913fbe2df5f670cbe6d1b</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QString</type>
      <name>toString</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a1e3cf04542e223ca5484ec631ac08791</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>RotateX</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a8bd3a012e34bc5adaa90c214f27dd337</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>RotateY</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a1165026cf7d37a820d31aa598723d5fc</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>RotateZ</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a8d8b5aa0726cdafd9358dbdabb8b449f</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>RotateAxisAngle</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>af24d8c6d52d98d5669c549040ea6b65d</anchor>
      <arglist>(const float3 &amp;axisDirection, float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>RotateFromTo</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a8de13a862a20e50d313c3a614c8c5553</anchor>
      <arglist>(const float3 &amp;sourceDirection, const float3 &amp;targetDirection)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromQuat</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ae670302d0ecd50660118304b42b27d9f</anchor>
      <arglist>(const Quat &amp;orientation)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromRS</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>afb8ea2879056bf14f9c4b3b30714fd87</anchor>
      <arglist>(const Quat &amp;rotate, const float3 &amp;scale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromRS</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>aceffb4855bc9dbc6ef10026d92ecdcb1</anchor>
      <arglist>(const float3x3 &amp;rotate, const float3 &amp;scale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromEulerXYX</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a48baee7cbfbcbae9f8bfd0e6eb5ce4ff</anchor>
      <arglist>(float ex, float ey, float ex2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromEulerXZX</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a1831c262a55eeccda157180794be98dd</anchor>
      <arglist>(float ex, float ez, float ex2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromEulerYXY</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a1fe7a5f7d05666e2803a1192b4c11c93</anchor>
      <arglist>(float ey, float ex, float ey2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromEulerYZY</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a22bc3752acc8af4005a9eeb9a42f67f8</anchor>
      <arglist>(float ey, float ez, float ey2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromEulerZXZ</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>aa981356a56a1a4f953467bb613e6c0ed</anchor>
      <arglist>(float ez, float ex, float ez2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromEulerZYZ</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ad441457142f0c3f7a3cf858645faa2b3</anchor>
      <arglist>(float ez, float ey, float ez2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromEulerXYZ</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a237a49ccaccd6622653366d021a47b46</anchor>
      <arglist>(float ex, float ey, float ez)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromEulerXZY</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a6727e3d4d0d2d18b55d983ddf55b358c</anchor>
      <arglist>(float ex, float ez, float ey)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromEulerYXZ</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a3416adb468bcf1e88400fdf71c1fb445</anchor>
      <arglist>(float ey, float ex, float ez)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromEulerYZX</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>acda161a3ccf4b95c1a70b11735a5c574</anchor>
      <arglist>(float ey, float ez, float ex)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromEulerZXY</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a00a499050a2dae60f24a024b42f910ed</anchor>
      <arglist>(float ez, float ex, float ey)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>FromEulerZYX</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>af69fe8ac054c72b99470d77ee4f6d884</anchor>
      <arglist>(float ez, float ey, float ex)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static ScaleOp</type>
      <name>Scale</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>adf647c55eae6af60543160dea0cc7806</anchor>
      <arglist>(float sx, float sy, float sz)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static ScaleOp</type>
      <name>Scale</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a3521973f4facf685cb069375b4562ce8</anchor>
      <arglist>(const float3 &amp;scale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>ScaleAlongAxis</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>af5a4f81777c4c4a126ad8e006bb0d299</anchor>
      <arglist>(const float3 &amp;axis, float scalingFactor)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static ScaleOp</type>
      <name>UniformScale</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a5625791e9fdfc24d3e342c2420da6461</anchor>
      <arglist>(float uniformScale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>ShearX</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ad507ed9fd38b9789748367cf8563413b</anchor>
      <arglist>(float yFactor, float zFactor)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>ShearY</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>aa31d0b178a95f77fd7e4b2ba3a0281d3</anchor>
      <arglist>(float xFactor, float zFactor)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>ShearZ</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>aaf3e9326e8b7f8067bb7035d5bcd31f0</anchor>
      <arglist>(float xFactor, float yFactor)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>Reflect</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a1ffc0a37d8aaff34a34309e5ea7d4d90</anchor>
      <arglist>(const Plane &amp;p)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>MakeOrthographicProjection</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>ae38c3423da25d662abe4a829f0edf973</anchor>
      <arglist>(float nearPlaneDistance, float farPlaneDistance, float horizontalViewportSize, float verticalViewportSize)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>MakeOrthographicProjection</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a69c8d07c9c99d6fb663ca3bb12dfa5aa</anchor>
      <arglist>(const Plane &amp;target)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>MakeOrthographicProjectionYZ</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>aba368eeb2a48782be5a4c972988e5149</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>MakeOrthographicProjectionXZ</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>abc83706a94dde6efe3f918feb9a74583</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x3</type>
      <name>MakeOrthographicProjectionXY</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a38995017543edc8391e5bd0657017984</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>v</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a17c3dc0b5f67d21835be2e852aa05cff</anchor>
      <arglist>[Rows][Cols]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float3x3</type>
      <name>zero</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a2a887df5e682e0a6ab083c886458cf47</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float3x3</type>
      <name>identity</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a5bc6edc4334c20bb2abf43a3dcf43336</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float3x3</type>
      <name>nan</name>
      <anchorfile>classfloat3x3.html</anchorfile>
      <anchor>a4f083c6d030056c9cbe1a861bcf02ede</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>float3x4</name>
    <filename>classfloat3x4.html</filename>
    <member kind="enumvalue">
      <name>Rows</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a4f9dedb1109531adcb989a0e965eadfaadf3efe067b9ab19a4f1bc45ff12d4617</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>Cols</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>acfde0d0012e1dd3679ae0aeb0fe5e8f3a4acc2db0840026ed19ef1a742d31a602</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3x4</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a79affaa72403dca21e61164d83f7162c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3x4</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a8be72e4b723f180decd71a4455bc4a9e</anchor>
      <arglist>(const float3x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3x4</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a2baf4debe9867904a7b43bb68b064b77</anchor>
      <arglist>(float _00, float _01, float _02, float _03, float _10, float _11, float _12, float _13, float _20, float _21, float _22, float _23)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3x4</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a67756cacb53dfff7ce3780767ac6e49f</anchor>
      <arglist>(const float3x3 &amp;other)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3x4</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a18c7e9f0156ad54c4fbfecbb45032a52</anchor>
      <arglist>(const float3x3 &amp;other, const float3 &amp;translate)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3x4</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a66acb685ada23813139f2a949f328d6e</anchor>
      <arglist>(const float3 &amp;col0, const float3 &amp;col1, const float3 &amp;col2, const float3 &amp;col3)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3x4</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>adc1387c3930bf6e40b35b3e82c371996</anchor>
      <arglist>(const Quat &amp;orientation)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float3x4</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ac2c76e6c7e4e7138530e27d57d79bd1d</anchor>
      <arglist>(const Quat &amp;orientation, const float3 &amp;translation)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>GetScale</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ae7243cb421c6518f7de52cafc9dc1f33</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>MatrixProxy&lt; Cols &gt; &amp;</type>
      <name>operator[]</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ac58dbcb5084e86c6a80f94d1e076ba79</anchor>
      <arglist>(int row)</arglist>
    </member>
    <member kind="function">
      <type>const MatrixProxy&lt; Cols &gt; &amp;</type>
      <name>operator[]</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a91cbcb7f122f7fc6f3d9b8c6f9f64dd5</anchor>
      <arglist>(int row) const </arglist>
    </member>
    <member kind="function">
      <type>float &amp;</type>
      <name>At</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a9bdc6582d1f8e7aba00279f815e292cb</anchor>
      <arglist>(int row, int col)</arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float</type>
      <name>At</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ae9c6eaa5244e82cb8d4b75a4e5fd5743</anchor>
      <arglist>(int row, int col) const </arglist>
    </member>
    <member kind="function">
      <type>float4 &amp;</type>
      <name>Row</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ac01bc0b6f89bb3bbca30fc1eae619730</anchor>
      <arglist>(int row)</arglist>
    </member>
    <member kind="function">
      <type>const float4 &amp;</type>
      <name>Row</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a6a69ff5ea69014700b9d4eab5b60e8c5</anchor>
      <arglist>(int row) const </arglist>
    </member>
    <member kind="function">
      <type>float3 &amp;</type>
      <name>Row3</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a7efe97d50d186b2df6c7bbb662ce8c41</anchor>
      <arglist>(int row)</arglist>
    </member>
    <member kind="function">
      <type>const float3 &amp;</type>
      <name>Row3</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ada3d9081fa9e3b379a9e170dd08f7612</anchor>
      <arglist>(int row) const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float3</type>
      <name>Col</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a31aaf7c2d91730690cded710b71bab2e</anchor>
      <arglist>(int col) const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float3</type>
      <name>Diagonal</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>accca8c66728fd926791a76d37621af5c</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ScaleRow3</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a7e4ff79410d6907ae3ddc78ff7b2beb1</anchor>
      <arglist>(int row, float scalar)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ScaleRow</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a895e5aaafa1224ea8b7bed65ba154faa</anchor>
      <arglist>(int row, float scalar)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ScaleCol</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a122e00d8d1f413b92a5b72d9cd02d48e</anchor>
      <arglist>(int col, float scalar)</arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float3x3</type>
      <name>Float3x3Part</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ab0e11cefb5455112b27d97648466e5bd</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float3</type>
      <name>TranslatePart</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>aaf40bca5c4fc148fa028d8e578ce8cba</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float3x3</type>
      <name>RotatePart</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ab6b8db723516bac3a64b513bff3e4d0a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>WorldX</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ac1e8c99fa3654d4170834d63a18a0688</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>WorldY</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a3cd135c3adc708481ce967b58f78606d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>WorldZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ade2d7816d5f3bfd12cb053ee77c07bfd</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float *</type>
      <name>ptr</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a207f119f30566161ebd4980e046e515a</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const float *</type>
      <name>ptr</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>af274f15d2421a57e309cd0505e84e31e</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRow</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a5d4564f87e86acd1c45662a9503b5f11</anchor>
      <arglist>(int row, const float3 &amp;rowVector, float m_r3)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRow</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a51da82cd2a52baff72bd97cd8e3e869a</anchor>
      <arglist>(int row, const float4 &amp;rowVector)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRow</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a8308cf215e491ff6aa0c1ae1fcc48f19</anchor>
      <arglist>(int row, const float *data)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRow</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ae78db8b9c540201cbe1f2ebdd68ec202</anchor>
      <arglist>(int row, float m_r0, float m_r1, float m_r2, float m_r3)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetCol</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>aa26e3ce539e6e56aa188df01a8a8f626</anchor>
      <arglist>(int column, const float3 &amp;columnVector)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetCol</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a63f025b7925a4501071a716ccdd42c95</anchor>
      <arglist>(int column, const float *data)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetCol</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a1c5bd08746a92517703d411b3eddd008</anchor>
      <arglist>(int column, float m_0c, float m_1c, float m_2c)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a2fc719a3b8d020b0f0f84aa720858f9d</anchor>
      <arglist>(float _00, float _01, float _02, float _03, float _10, float _11, float _12, float _13, float _20, float _21, float _22, float _23)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a524f83eeff9c529af3c44305ba4dfc1b</anchor>
      <arglist>(const float3x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a6d2eca99245b95c2213a2d99638af4aa</anchor>
      <arglist>(const float *values)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>adfb23e3095ae0d25b57fad474079c892</anchor>
      <arglist>(int row, int col, float value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3Part</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ad983d2e4a5444da65a69eee7bbeb72dd</anchor>
      <arglist>(const float3x3 &amp;rotation)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetIdentity</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>aeaa1952b51cc351a15e9ff10f97a770d</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SwapColumns</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a7769ff0b71ea64100c20342d6eff77d8</anchor>
      <arglist>(int col1, int col2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SwapRows</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a6ca8a19f0c64a5e4f18aeaeeaa938469</anchor>
      <arglist>(int row1, int row2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetTranslatePart</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a51e26416501f64e6b44eda1f31eaa23d</anchor>
      <arglist>(float tx, float ty, float tz)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetTranslatePart</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>aa20fdf51bd1af70c691c8224f0a628a1</anchor>
      <arglist>(const float3 &amp;offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePartX</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ac4c066dedf4cb60b96d24a348a784b70</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePartY</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ac5ad8c6049413ba95dcc54161de9b6d3</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePartZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a2ea90ff83d8ef5be94d6179356b5746b</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePart</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>aef695f088c2c582124d9d1ba7eb6a9bf</anchor>
      <arglist>(const float3 &amp;axisDirection, float angleRadians)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePart</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a13f1b88086730e25a503e7dcf53f873d</anchor>
      <arglist>(const Quat &amp;orientation)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePart</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ae8f7c86974dc5e9c2f2ff0eccdb93885</anchor>
      <arglist>(const float3x3 &amp;rotation)</arglist>
    </member>
    <member kind="function">
      <type>float3x4 &amp;</type>
      <name>operator=</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a57bd74672b2c386cc0fa242390dee413</anchor>
      <arglist>(const float3x3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x4 &amp;</type>
      <name>operator=</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a0959853b3e0b2678528418297553539c</anchor>
      <arglist>(const Quat &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x4 &amp;</type>
      <name>operator=</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a1125f7f818ce089840de78db1bc4b5ec</anchor>
      <arglist>(const float3x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Determinant</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a618ae97effec0ad0388c62b3861aac34</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Inverse</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>af6df80ece047e9accc7b8797a263d4ba</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>Inverted</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a2b30c6366f7be25d7718726564e966da</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>InverseOrthogonal</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a6ca18508475c10319da40abfb6afe6c6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>InverseOrthogonalUniformScale</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ab58686dcf50e74fd821ecebc15fbf750</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>InverseOrthonormal</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a4c3a4d3e29f367822fcece2697f81551</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transpose3</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a3fbe881221759b8a6b9feee7119392a1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>Transposed3</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ac040a5e21352949f92b2a0f8ebf3341a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>InverseTranspose</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ac5191f325f2e093a5f8f4b9046ffce09</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>InverseTransposed</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ab63d342c3eea8903e304680d4f028f26</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Trace</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a73b44325664b3c1ecc8f874a31ce6b4c</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Orthonormalize</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a70ef07978d2eb5b5bfe4139c15cfdd12</anchor>
      <arglist>(int firstColumn, int secondColumn, int thirdColumn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RemoveScale</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a058609b1fb0891e6bb705c8bf4131162</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>TransformPos</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>abef603125e904d86ea102dadf2cc90c5</anchor>
      <arglist>(const float3 &amp;pointVector) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>TransformPos</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>af905d08c00bde077281f829be402f6ca</anchor>
      <arglist>(float x, float y, float z) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>TransformDir</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a0dec584ca37ec2fade861431641cdb65</anchor>
      <arglist>(const float3 &amp;directionVector) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>TransformDir</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a070b8ba065fefc498f70d871a850e149</anchor>
      <arglist>(float x, float y, float z) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Transform</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a4da5be38f080e85c021b7f44b9b26061</anchor>
      <arglist>(const float4 &amp;vector) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>BatchTransformPos</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>acce2ebc21d464effb778ff7382d0cd73</anchor>
      <arglist>(float3 *pointArray, int numPoints) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>BatchTransformPos</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a4eb21109cf589455a221ce707fbb7971</anchor>
      <arglist>(float3 *pointArray, int numPoints, int stride) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>BatchTransformDir</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a8540a4dd7bdb8983ac627b1529f303b1</anchor>
      <arglist>(float3 *dirArray, int numVectors) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>BatchTransformDir</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ae3488f9a9b8af779c831dd3fac05353a</anchor>
      <arglist>(float3 *dirArray, int numVectors, int stride) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>BatchTransform</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a8c36e4fc229af305514e1e5756a53710</anchor>
      <arglist>(float4 *vectorArray, int numVectors) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>BatchTransform</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a96e8947d59528c00a863bff5fa477654</anchor>
      <arglist>(float4 *vectorArray, int numVectors, int stride) const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>af922cfa2f82f1a911453c595726b36e0</anchor>
      <arglist>(const float3x3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a2380154e1cfad3b44f1fced3dddd5cb1</anchor>
      <arglist>(const float3x4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a2abe80c35e1c05b46fbc4775683ce93d</anchor>
      <arglist>(const Quat &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>operator*</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>afaa7842505c5605413dd78e2f6de1b97</anchor>
      <arglist>(const float4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator*</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a5d6cf32a0f05d08b2af88b9d17c8d2f4</anchor>
      <arglist>(float scalar) const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator/</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a9397d76a0b87c1a2e5280e38cb825c11</anchor>
      <arglist>(float scalar) const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator+</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a2f0f399fcd5ad43d3646a5345a563e94</anchor>
      <arglist>(const float3x4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator-</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>aed1a3203f475da96698fd6ab1d2631d9</anchor>
      <arglist>(const float3x4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>operator-</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a8f60356a047860a405f78aa95f84ebad</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3x4 &amp;</type>
      <name>operator*=</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a33e4ae07430de3badb398a10c8ceddeb</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>float3x4 &amp;</type>
      <name>operator/=</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a0f39f351f9d9fb55c342a8a2b22f9004</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>float3x4 &amp;</type>
      <name>operator+=</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a5d8c51f43543c81748a1ec775b9bd443</anchor>
      <arglist>(const float3x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float3x4 &amp;</type>
      <name>operator-=</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ab4658f0012a4e784da103e2860669ffe</anchor>
      <arglist>(const float3x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFinite</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>af3eb1d8d334ba3d3cc0a7d1e41db07dc</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsIdentity</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a9acc8011303871bac6aade3320427bb4</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsLowerTriangular</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a11a2965303e77b9db0b41b6de16485ee</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsUpperTriangular</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a08a4245fa19e427f4c6b5296b38ccb17</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsInvertible</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>abdbf0b282c758a2d9a631b08f229d31b</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsSymmetric</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a596cb0e9847492894a124bbe4408763e</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsSkewSymmetric</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a353f485959cfbb71fefc99c2453c5cee</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>HasUnitaryScale</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ae2355482006873855d45367d7705a409</anchor>
      <arglist>(float epsilonSq=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>HasNegativeScale</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a817176d9321f9d74719e442b00ff5ee6</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>HasUniformScale</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a0c8ff8696fdc56e11cb2b2d14e24ea93</anchor>
      <arglist>(float epsilonSq=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsOrthogonal</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ae7f3e28382c9f3e07b3db4b0d72d82e7</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsOrthonormal</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ae0d1c715c0d8e43eb11e1a3658b30031</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Equals</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a3ec1c39aeedceab6ee3c86ce0673fcd7</anchor>
      <arglist>(const float3x4 &amp;other, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXYX</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a8224cd22c30794970acccb0a6e57ea3a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXZX</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a99f7566df0c9422b9a102be7c9699899</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYXY</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a5db68f39af688e2593acdc0121a61871</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYZY</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a26f563de7021f969911e2512bbf6cac5</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZXZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a45d962ac148cf64c96ce520f4c8c1b9b</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZYZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a64e48f55b5928e365421b3c5e7410d50</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXYZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a64a338b185a8f010c26d3fde1c5ccd06</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXZY</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ad024e8c6aa3b159ed21be4e6be517c4f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYXZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a15e3bfc6dfd5787a61cd3ec520c190ce</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYZX</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ab9810691d2ae1ca173aaec81875682ba</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZXY</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a4d7b9f33421aca9700c66682c184e87a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZYX</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ae8cdcb17837ddbe26371fba96e8456d3</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ExtractScale</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ac9d06b8af31eb16f5b907b5b1a8a1c54</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Decompose</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a7cb76970ddb27d44b6ef70fa1292df18</anchor>
      <arglist>(float3 &amp;translate, Quat &amp;rotate, float3 &amp;scale) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Decompose</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a44867811a03274d4957c4dfe4600f231</anchor>
      <arglist>(float3 &amp;translate, float3x3 &amp;rotate, float3 &amp;scale) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Decompose</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a1ed2f0748fb4f3ed7358416b856233f2</anchor>
      <arglist>(float3 &amp;translate, float3x4 &amp;rotate, float3 &amp;scale) const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>Mul</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a5b40b3a597fa7df1d579002cf3227551</anchor>
      <arglist>(const float3x3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>Mul</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a96f2814129586b54f6be3594bdca73ed</anchor>
      <arglist>(const float3x4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>Mul</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a98d4147e28c453a9df9e47394048e725</anchor>
      <arglist>(const float4x4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>Mul</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a52fabf40db8781854930d91196628602</anchor>
      <arglist>(const Quat &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>MulPos</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ad5457d40805c967738b22c41eecb2d88</anchor>
      <arglist>(const float3 &amp;pointVector) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>MulDir</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a842347b155dcf1b120e306ea0aa3e170</anchor>
      <arglist>(const float3 &amp;directionVector) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Mul</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>abf59622868c98223300a2e2ca4a5655a</anchor>
      <arglist>(const float4 &amp;vector) const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator QString</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ace5cf7d28046d785d10e60c8dd75abcb</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QString</type>
      <name>toString</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a9df32998b14f74efa0d83a665e2cf100</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function" static="yes">
      <type>static TranslateOp</type>
      <name>Translate</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a521a8e17c291c00343701f4d8b26629b</anchor>
      <arglist>(float tx, float ty, float tz)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static TranslateOp</type>
      <name>Translate</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a50a753e0330f47bdd93365a97bde15a9</anchor>
      <arglist>(const float3 &amp;offset)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>RotateX</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a569f1aef595184eb374e9625bb6390fd</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>RotateX</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>af025f87ef0b95a14d675dc35ba220d47</anchor>
      <arglist>(float angleRadians, const float3 &amp;pointOnAxis)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>RotateY</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a8afeee78f69ebfb34dabe2e6914003d1</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>RotateY</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a68b4c694e0614092588d04a40474e772</anchor>
      <arglist>(float angleRadians, const float3 &amp;pointOnAxis)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>RotateZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a0dbf44f8eb54de3c16b464bc39a865d4</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>RotateZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a96aa12d61a9d1f6ba4593ca350220104</anchor>
      <arglist>(float angleRadians, const float3 &amp;pointOnAxis)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>RotateAxisAngle</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>adbb50a6d415de0eb79b86ef7a09cd890</anchor>
      <arglist>(const float3 &amp;axisDirection, float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>RotateAxisAngle</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a3c5607e278931d2b89c5c3bc40209c16</anchor>
      <arglist>(const float3 &amp;axisDirection, float angleRadians, const float3 &amp;pointOnAxis)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>RotateFromTo</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a3a4a488e01014f6998661f9bf8c19c27</anchor>
      <arglist>(const float3 &amp;sourceDirection, const float3 &amp;targetDirection)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>RotateFromTo</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a3592b11d95456892c7a2a48e034037db</anchor>
      <arglist>(const float3 &amp;sourceDirection, const float3 &amp;targetDirection, const float3 &amp;centerPoint)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromQuat</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>aba01c2b5dc131e0526694fb6c13c2164</anchor>
      <arglist>(const Quat &amp;orientation)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromQuat</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>afe0a0fde71489415737853faf8d79cdf</anchor>
      <arglist>(const Quat &amp;orientation, const float3 &amp;pointOnAxis)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromTRS</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a23a9ebd2c002b6e3aff866ee945b5be8</anchor>
      <arglist>(const float3 &amp;translate, const Quat &amp;rotate, const float3 &amp;scale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromTRS</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a0b68ea260cd23bd148d972b1d12b3867</anchor>
      <arglist>(const float3 &amp;translate, const float3x3 &amp;rotate, const float3 &amp;scale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromTRS</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>af78a2e1d6caadb79757f4f893ca7aa7d</anchor>
      <arglist>(const float3 &amp;translate, const float3x4 &amp;rotate, const float3 &amp;scale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromEulerXYX</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a2382bafb0dbe505a8d7866bc0f2a29fc</anchor>
      <arglist>(float ex, float ey, float ex2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromEulerXZX</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a6d769e7ab5c3167cab7528a866f8613a</anchor>
      <arglist>(float ex, float ez, float ex2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromEulerYXY</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>aca9b8b84053a47558c57b9a217957949</anchor>
      <arglist>(float ey, float ex, float ey2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromEulerYZY</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>aa9f5d931b9ac9b312b0079c5cd21a296</anchor>
      <arglist>(float ey, float ez, float ey2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromEulerZXZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a1b9ad54dc21caf16b08092114e378134</anchor>
      <arglist>(float ez, float ex, float ez2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromEulerZYZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a2a52b09786ef0b64d5ef7740bdbb84d8</anchor>
      <arglist>(float ez, float ey, float ez2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromEulerXYZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a9bfa7718187b62c00cea7d850a8cf6ab</anchor>
      <arglist>(float ex, float ey, float ez)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromEulerXZY</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>aac5849ddeab2ddd4fb48224ebbcf2503</anchor>
      <arglist>(float ex, float ez, float ey)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromEulerYXZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a0aefdcafac165d1e801207cfb1a2776e</anchor>
      <arglist>(float ey, float ex, float ez)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromEulerYZX</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>adf5dbdab27eef589d4becde21a52486f</anchor>
      <arglist>(float ey, float ez, float ex)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromEulerZXY</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ae9bf92f3bfae8553312e3d57979abb6b</anchor>
      <arglist>(float ez, float ex, float ey)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>FromEulerZYX</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a6b3ba6b62b82dfbd74f50a6bb45881ee</anchor>
      <arglist>(float ez, float ey, float ex)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static ScaleOp</type>
      <name>Scale</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a9b69b49bf6255454f7e1d536c698b2af</anchor>
      <arglist>(float sx, float sy, float sz)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static ScaleOp</type>
      <name>Scale</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a498218ea77f5434b242f81b10f68adf0</anchor>
      <arglist>(const float3 &amp;scale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>Scale</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a98699527a6d28985e425a0c4c02d207c</anchor>
      <arglist>(const float3 &amp;scale, const float3 &amp;scaleCenter)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>ScaleAlongAxis</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>aec3e38abe21e1aa9dd72a074303d8281</anchor>
      <arglist>(const float3 &amp;axis, float scalingFactor)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>ScaleAlongAxis</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>af8da9f30a0116e2409ceda5cb4f5ddeb</anchor>
      <arglist>(const float3 &amp;axis, float scalingFactor, const float3 &amp;scaleCenter)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static ScaleOp</type>
      <name>UniformScale</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a3a45b16ccef7a6c6b2f322220bff5617</anchor>
      <arglist>(float uniformScale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>ShearX</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a99626d83d58aad2dc388b05867041c82</anchor>
      <arglist>(float yFactor, float zFactor)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>ShearY</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a9383fcfef5c4423e635c8a86e6f8ecc5</anchor>
      <arglist>(float xFactor, float zFactor)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>ShearZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a84dee3c8480bd64da6122104c4c7a4ff</anchor>
      <arglist>(float xFactor, float yFactor)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>Reflect</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a79babab4c18ea234a9487891232ea7d6</anchor>
      <arglist>(const Plane &amp;p)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>MakeOrthographicProjection</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>adb0021c1bfee41f3d3cccd0040ab2ac6</anchor>
      <arglist>(float nearPlaneDistance, float farPlaneDistance, float horizontalViewportSize, float verticalViewportSize)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>MakeOrthographicProjection</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a696ad64afbfe992dbf47b6d647456900</anchor>
      <arglist>(const Plane &amp;target)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>MakeOrthographicProjectionYZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>ae61f51350f8c657049e48859ef0ef70a</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>MakeOrthographicProjectionXZ</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a66272f777368a32e1a85ed9cdf33ea33</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>MakeOrthographicProjectionXY</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>aa8222d5001399e8830a923cd11cc4fb2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float3x4</type>
      <name>LookAtRH</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a71a7a1d1eb84ae498d1bc49ebdde00f5</anchor>
      <arglist>(const float3 &amp;localForwardDir, const float3 &amp;targetForwardDir, const float3 &amp;localUp, const float3 &amp;worldUp)</arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>v</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>addbcc65791fb7930821e8faed2cf7955</anchor>
      <arglist>[Rows][Cols]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float3x4</type>
      <name>zero</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a96f7537edc5f05fba9eb8e07a3f2772f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float3x4</type>
      <name>identity</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>a55fa8a6b65fe3bf1738c35a75e7f582a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float3x4</type>
      <name>nan</name>
      <anchorfile>classfloat3x4.html</anchorfile>
      <anchor>aa1c0edf0a60c79ad6482bb47c082e147</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>float4</name>
    <filename>classfloat4.html</filename>
    <member kind="enumvalue">
      <name>Size</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a411199337e75dbad94d2bc41fc68492aa42b4159e202e5c969688762e3f9b18b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a219529d0e59732edc22565b299bd4bd9</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>afe0923ded8bc658f02501ab8a7ed468f</anchor>
      <arglist>(const float4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>aa97386d9a7f4bdddae47aab809cb3135</anchor>
      <arglist>(float x, float y, float z, float w)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ab61db90c08426a76c4f632ae9b109452</anchor>
      <arglist>(const float3 &amp;xyz, float w)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>aba96b51ac1f9bf8563c22f06741735a2</anchor>
      <arglist>(const float *data)</arglist>
    </member>
    <member kind="function">
      <type>float *</type>
      <name>ptr</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>aa5460804e2281587801cd2734db89d54</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const float *</type>
      <name>ptr</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a90a3887351e56d95da9f9b4584df5ea0</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float</type>
      <name>operator[]</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a9ac89257eda7e5e0c3a4016a16df42af</anchor>
      <arglist>(int index) const </arglist>
    </member>
    <member kind="function">
      <type>float &amp;</type>
      <name>operator[]</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>af11fb67ed651a16fa1bd29e14b9c71c1</anchor>
      <arglist>(int index)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>xyz</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>af1bf444057acf5f78cc51cc7ada72886</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>LengthSq3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a4315a03a0582afe7919c4ea09e40ad87</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Length3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a3541ddd8abaddfb0b6f9f9040015ef04</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>LengthSq4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a81b9cc90f1ec052bdc0c1b06847890b7</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Length4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ad6195a57dba511e26062ef4a63722781</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Normalize3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>aef9eef15376c6192c8c5a830a538d144</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Normalize4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>add254b76468d66ca05f5a4377a3cc363</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Normalized3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ad004ee50b6bb7f94867495df1641ccc6</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Normalized4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a94e33dbdebcef92f9c2dd98a1f39c78f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>NormalizeW</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a0902195830609c28f5df41611284ab85</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsWZeroOrOne</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>abc6709b1eb9bc845131aef55b6b585d2</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsZero3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a64a6c8a0975c8167c3a8076e6c15a5ba</anchor>
      <arglist>(float epsilonSq=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsZero4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ab8079fcd6bfa8dc78aa2b4fc37b1b79c</anchor>
      <arglist>(float epsilonSq=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsNormalized3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a234900263ae738299f056a0843249878</anchor>
      <arglist>(float epsilonSq=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsNormalized4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>aedb7c1cbbb373a35e86e16de2d047b6d</anchor>
      <arglist>(float epsilonSq=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Scale3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a41d21c4f9eb6d9f12516def32964ccde</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>ScaleToLength3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a5609fb7dfd74e96c1d6f6dd25cf6ad4b</anchor>
      <arglist>(float newLength)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>ScaledToLength3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a2b7eeeecb4e1df2888429db66da9bfb3</anchor>
      <arglist>(float newLength) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFinite</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ab59d2a7431110c20c0d74f297253e284</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsPerpendicular3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a3acbe5220e2656f335e5746052a7eb0c</anchor>
      <arglist>(const float4 &amp;other, float epsilon=1e-6f) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>SumOfElements</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>acfefbb8359c0785d34af07bf3ca19641</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>ProductOfElements</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a3fab06453bb4d596c11ecd76da011dd4</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>AverageOfElements</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>af401677582f0363127b56d55f2cc0927</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>MinElement</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a18d5cadc0601c27cbfccd2450e173a19</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>MinElementIndex</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a8265745f8f18d12b3235f2798885d0e3</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>MaxElement</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>aa4379fa56e8d61abbf6cc6578fcbd16f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>MaxElementIndex</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a941da4505ad42f731ae73344c7d168ea</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Abs</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a487df689b41ff295a9235aee89332211</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Min</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a90d090c20d7eef4a1ded5d4a58653782</anchor>
      <arglist>(float ceil) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Min</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a42e4ed56791e70a291546c812b8c2d1f</anchor>
      <arglist>(const float4 &amp;ceil) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Max</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ac5b58d0ded7d5f5f9ab300ddad8b110b</anchor>
      <arglist>(float floor) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Max</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a8b78c5e648628fabef4c43b9d7c5b26c</anchor>
      <arglist>(const float4 &amp;floor) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Clamp</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a2c3a3365ba57baf21a980795a23b364b</anchor>
      <arglist>(const float4 &amp;floor, const float4 &amp;ceil) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Clamp01</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a6af8d070553d1ec574ced70f27a63676</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Clamp</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ab3b71826c6ba1af7fa27cb3d207f925a</anchor>
      <arglist>(float floor, float ceil) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Lerp</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>af153d72bc1ad0fd4ba7c6c292dfeddf3</anchor>
      <arglist>(const float4 &amp;b, float t) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance3Sq</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a472ec38c7b8862d35b24fe8ac4b7cf11</anchor>
      <arglist>(const float4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ab3681b001810934c4c64f4fa49092fb2</anchor>
      <arglist>(const float4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Dot3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ab66d34374fd69a8dbe7d30c7d2d54151</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Dot3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ab62783a97a2116996061ed28ba1663f4</anchor>
      <arglist>(const float4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Dot4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a36c118ed9080d87a4374a1d30051ca30</anchor>
      <arglist>(const float4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Cross3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a5f7b9b812015196b1682deeb9894393b</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Cross3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a80595361fd55c62645ec8b0781d7a81a</anchor>
      <arglist>(const float4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>OuterProduct</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a551535caba59872daa559918c27c58c9</anchor>
      <arglist>(const float4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Perpendicular3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a8d9f66cd0a7cb45612cd6476e91c4cec</anchor>
      <arglist>(const float3 &amp;hint=float3(0, 1, 0), const float3 &amp;hint2=float3(0, 0, 1)) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>AnotherPerpendicular3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a10a5f82a7af3ff6d9fc7e2b0f8e5df68</anchor>
      <arglist>(const float3 &amp;hint=float3(0, 1, 0), const float3 &amp;hint2=float3(0, 0, 1)) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Reflect3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a8504694ef5f7a0a8096a11a8248a6a99</anchor>
      <arglist>(const float3 &amp;normal) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>AngleBetween3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a0174c61abe1b2237867bc8cae4ae2bc9</anchor>
      <arglist>(const float4 &amp;other) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>AngleBetweenNorm3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a5d2f397690c5c66a82295530e4372838</anchor>
      <arglist>(const float4 &amp;normalizedVector) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>AngleBetween4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a9521866fe36ee8bed793bc8adf7c747d</anchor>
      <arglist>(const float4 &amp;other) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>AngleBetweenNorm4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ab857ef5e2ad612eb842f8d717222053d</anchor>
      <arglist>(const float4 &amp;normalizedVector) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>ProjectTo3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a310ac66adc50f15cd7f2b222abf30f32</anchor>
      <arglist>(const float3 &amp;target) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>ProjectToNorm3</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>adae2a316d75f88d0e837642ace01c200</anchor>
      <arglist>(const float3 &amp;target) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFromScalar</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a9c7a3c0be0ea802325c44e54b3f555d7</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFromScalar</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a50823285a87c7d69802d47a75205de01</anchor>
      <arglist>(float scalar, float w)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a24d5eb811182ba828f132d6418aa5e07</anchor>
      <arglist>(float x, float y, float z, float w)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Equals</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>af28a152ac60d1e4fd3b7fc285562da35</anchor>
      <arglist>(const float4 &amp;other, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Equals</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a2b21789b937b9026fa87ffe6dbb99e72</anchor>
      <arglist>(float x, float y, float z, float w, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>operator+</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>aead48d897b8cbecfaaba44aff814124a</anchor>
      <arglist>(const float4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>operator-</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ad98a1735ec90c4cb9d075beb4c4c45ca</anchor>
      <arglist>(const float4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>operator-</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ae6a8e62f361592adbc3b4decaf4b8494</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>operator*</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a7c56180115509cf4d518c0031554a362</anchor>
      <arglist>(float scalar) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>operator/</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a75323e55f020b57f9a454fe3dc00c0cc</anchor>
      <arglist>(float scalar) const </arglist>
    </member>
    <member kind="function">
      <type>float4 &amp;</type>
      <name>operator+=</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a80b4426f80e35ede3afd87cc4b8e4f31</anchor>
      <arglist>(const float4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4 &amp;</type>
      <name>operator-=</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ab0dfdcd297cc60a14e741e2fcc7103be</anchor>
      <arglist>(const float4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4 &amp;</type>
      <name>operator*=</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>aa301e342a6a2ab0a10a6c3e5512af027</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>float4 &amp;</type>
      <name>operator/=</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a69507306e8ea6eebf6016ee0de50fc13</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Add</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a2e2641f6f8178cbb5364d5fe59aacb83</anchor>
      <arglist>(const float4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Sub</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>aa777f039f595ba656609b6b61564ff1a</anchor>
      <arglist>(const float4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Mul</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a2dba5a719d74e1faf36b95fb6234e8c2</anchor>
      <arglist>(float rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Div</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a2659a6076921a49f72d156e8600a8458</anchor>
      <arglist>(float rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Neg</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a6a526f3688f141d2d39acbf956b7aef5</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Mul</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a0a4a1e184eb885dea1ed084f048c514b</anchor>
      <arglist>(const float4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a7591249b5e88026a91c2c039b1d49fdd</anchor>
      <arglist>(const QVector4D &amp;other)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator QVector4D</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a9560a9267e5cb38b6d238f89b44ff3c2</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator QString</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>aa4bf534a071c13f8b40a8a372fedb885</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QString</type>
      <name>toString</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a2324b5954e8ed8825f0475ec20982b02</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QVector4D</type>
      <name>ToQVector4D</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a3cc23a6954f64acc9e1e5a2ee85e50b4</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4</type>
      <name>FromString</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a61eb26f95374d31286dbbb818ae6f05d</anchor>
      <arglist>(const char *str)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4</type>
      <name>Lerp</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a5f3a288bd20b5b56ab3c399bdeb655d6</anchor>
      <arglist>(const float4 &amp;a, const float4 &amp;b, float t)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4</type>
      <name>FromScalar</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a511fe23051f9a78012ec3f02be1c1861</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4</type>
      <name>FromScalar</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>af9243193ed90c7a509cdd1e9a533a4ba</anchor>
      <arglist>(float scalar, float w)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4</type>
      <name>FromQVector4D</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>aa84bf2223b292b5eb87ab590d492e307</anchor>
      <arglist>(const QVector4D &amp;v)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4</type>
      <name>FromString</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a0aada179d928a44919f78a83303e7374</anchor>
      <arglist>(const QString &amp;str)</arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>x</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ab60e7164162636ef8d0315cf18269b0f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>y</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a72bce4984c96cef3d6a50adcbce6e388</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>z</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a898443ac0132ee9f149661c63882361b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>w</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>ae3ef7af10eec7945566a363866321e26</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float4</type>
      <name>zero</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>abac3cf6db09fe4be8390e5da1b376686</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float4</type>
      <name>one</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a4a28ac2a542b702c9350ac8f4cfc2b87</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float4</type>
      <name>unitX</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a274013fcd738badb67779af137af9377</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float4</type>
      <name>unitY</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a78a2297be3e3bb3190cebd730a481f68</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float4</type>
      <name>unitZ</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a2d0b5dc13e7053ba1a83ea01249ec179</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float4</type>
      <name>unitW</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a77539ef5255e0f793763f80fefaf88c4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float4</type>
      <name>nan</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>a3999dc630b47bfbdeb2b7d25a5c19580</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float4</type>
      <name>inf</name>
      <anchorfile>classfloat4.html</anchorfile>
      <anchor>afc053a8fc35bf8c00dcac4700a0be71d</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>float4x4</name>
    <filename>classfloat4x4.html</filename>
    <member kind="enumvalue">
      <name>Rows</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a01119ab1850302423af5ce1b370290a3aa8d6d35c6e9b8a0cda2c5984bc64d7f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>Cols</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a4608bd5cf2eb6481bf35e8062ae5bb3da2a8696afc2b6c902a5cca93da1a7b6a5</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4x4</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a574b76a810a60fbeceac3198ea29f0b7</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4x4</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a95182cad9deebb17de7713eccf4a3ec0</anchor>
      <arglist>(const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4x4</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a6038636175bfced10c2e75b6d25dc7a1</anchor>
      <arglist>(float _00, float _01, float _02, float _03, float _10, float _11, float _12, float _13, float _20, float _21, float _22, float _23, float _30, float _31, float _32, float _33)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4x4</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ad0d8289d3f0a3c6dc78f9f18214c07be</anchor>
      <arglist>(const float3x3 &amp;other)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4x4</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ae7fe09b514296d05843e33abb23ed4f1</anchor>
      <arglist>(const float3x4 &amp;other)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4x4</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a38b3c842f80dcf5dcc9ef9ff8898b67b</anchor>
      <arglist>(const float4 &amp;col0, const float4 &amp;col1, const float4 &amp;col2, const float4 &amp;col3)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4x4</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ac6845e6eaac4bd8a5941d3a426d015fc</anchor>
      <arglist>(const Quat &amp;orientation)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4x4</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a1eb96b774da3edc9cc71bfc5b6e2a5a7</anchor>
      <arglist>(const Quat &amp;orientation, const float3 &amp;translation)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>GetScale</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a06f270da27240abfca5283470d6a09f7</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>ComplementaryProjection</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a5fff46987cdeac18d9ffdfa208d78d76</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>MatrixProxy&lt; Cols &gt; &amp;</type>
      <name>operator[]</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a867cbac6896a5386ec70171e219592e4</anchor>
      <arglist>(int row)</arglist>
    </member>
    <member kind="function">
      <type>const MatrixProxy&lt; Cols &gt; &amp;</type>
      <name>operator[]</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>abf70e4bf857f6638862bd095a9e15789</anchor>
      <arglist>(int row) const </arglist>
    </member>
    <member kind="function">
      <type>float &amp;</type>
      <name>At</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ae2a6f01176daa956c3c324d72b098f74</anchor>
      <arglist>(int row, int col)</arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float</type>
      <name>At</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a2d7e9762a919e0c227b4c2543be26426</anchor>
      <arglist>(int row, int col) const </arglist>
    </member>
    <member kind="function">
      <type>float4 &amp;</type>
      <name>Row</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>acc6e9bddcac357ab55c70db26ea2e566</anchor>
      <arglist>(int row)</arglist>
    </member>
    <member kind="function">
      <type>const float4 &amp;</type>
      <name>Row</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a761fa2762f6c8644393026f5cc7df568</anchor>
      <arglist>(int row) const </arglist>
    </member>
    <member kind="function">
      <type>float3 &amp;</type>
      <name>Row3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a622cf8f799ea9a8ecb777e5ca84e50be</anchor>
      <arglist>(int row)</arglist>
    </member>
    <member kind="function">
      <type>const float3 &amp;</type>
      <name>Row3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ad49c0e22a026423ffdad6c8d3553903b</anchor>
      <arglist>(int row) const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float4</type>
      <name>Col</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a79d9c483b96abaffdf1eb3559c570013</anchor>
      <arglist>(int col) const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float3</type>
      <name>Col3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a3376e3cede84730d3db5425250f56a28</anchor>
      <arglist>(int col) const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float4</type>
      <name>Diagonal</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ab68da37bbb46b8c7f4c7cd3ca63afd74</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float3</type>
      <name>Diagonal3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a4529f851efc86552a6738e167790dfb4</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ScaleRow3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a48db26b594ca0576787cf7733f129cbf</anchor>
      <arglist>(int row, float scalar)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ScaleRow</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a2b88ef8ec300f521f692bb2e9d3e9096</anchor>
      <arglist>(int row, float scalar)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ScaleCol3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a4b711fcb7731b1d1522ba1cc16fccf7b</anchor>
      <arglist>(int row, float scalar)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ScaleCol</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a3768db63bd13b9880d0678ea2f67f87b</anchor>
      <arglist>(int col, float scalar)</arglist>
    </member>
    <member kind="function">
      <type>const float3x3</type>
      <name>Float3x3Part</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a9abfc453047cb5a866e5ccddf030b877</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3x4 &amp;</type>
      <name>Float3x4Part</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a806914747efc53351a505027e6c1ff55</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const float3x4 &amp;</type>
      <name>Float3x4Part</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aae082989c21b375ee31041eb170bc914</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float3</type>
      <name>TranslatePart</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a238cbc73a8402d07ba8da6fca8c5c4a6</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>CONST_WIN32 float3x3</type>
      <name>RotatePart</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>abbaf24c29c59cf072a01fca0f6499c26</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>WorldX</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a59d58d0bed4c19997eddf63fb434e047</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>WorldY</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a8c3903985e23efae818e0421ae5a67a5</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>WorldZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aa1d6f39021e037721b15c5343b44b261</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float *</type>
      <name>ptr</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a883a30aae6a510e2423de7bd22dc8627</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const float *</type>
      <name>ptr</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a2201fc87bc756a0540ef8bcee4e9306b</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRow3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a1ee6321e95432e7e4e467cd5169a340c</anchor>
      <arglist>(int row, const float3 &amp;rowVector)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRow3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a8f44e983b75dc8afed926015b88aeb84</anchor>
      <arglist>(int row, const float *data)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRow3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aa11681f87057e2aff27736d9bd6239a5</anchor>
      <arglist>(int row, float m_r0, float m_r1, float m_r2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRow</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ada52608c749716f3da4b627b560e70ad</anchor>
      <arglist>(int row, const float3 &amp;rowVector, float m_r3)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRow</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a773d7b0a86647384213e162bd2425667</anchor>
      <arglist>(int row, const float4 &amp;rowVector)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRow</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a3bcf390e2f3089e608e21f63e22121af</anchor>
      <arglist>(int row, const float *data)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRow</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a2ba21c88e6a92fb4158fc2881c3ba95a</anchor>
      <arglist>(int row, float m_r0, float m_r1, float m_r2, float m_r3)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetCol3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a438e67140ba2405f3ad987b0d4945723</anchor>
      <arglist>(int column, const float3 &amp;columnVector)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetCol3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a0b361bdf2bcdd69c8deade6ac61ff385</anchor>
      <arglist>(int column, const float *data)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetCol3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ac98f14f5a429c755826d5f135df64f6a</anchor>
      <arglist>(int column, float m_0c, float m_1c, float m_2c)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetCol</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a6bf807b4a5ca38966acc21af4b0e37a8</anchor>
      <arglist>(int column, const float3 &amp;columnVector, float m_3c)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetCol</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a01ba46f4c5f636c996d27e7857606e53</anchor>
      <arglist>(int column, const float4 &amp;columnVector)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetCol</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a69c643149cbfa8e1ec0af6996fa1e2bb</anchor>
      <arglist>(int column, const float *data)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetCol</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a82f44d647b04d13203c95a7818bcfdf5</anchor>
      <arglist>(int column, float m_0c, float m_1c, float m_2c, float m_3c)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a9003acd4cce6b193aba6c683161346c7</anchor>
      <arglist>(float _00, float _01, float _02, float _03, float _10, float _11, float _12, float _13, float _20, float _21, float _22, float _23, float _30, float _31, float _32, float _33)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ab2f5b9718b43d80b4d821228dc1cc25a</anchor>
      <arglist>(const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a2914f20ab6c729808f4f379315906db7</anchor>
      <arglist>(const float *values)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ab90248bb5d1791da8e7c0635db5c0b73</anchor>
      <arglist>(int row, int col, float value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x3Part</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a36432af85c1f750489f6b9b93c94fc16</anchor>
      <arglist>(const float3x3 &amp;rotation)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set3x4Part</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a6acf8ed3975103e0ba4ce805b9716a70</anchor>
      <arglist>(const float3x4 &amp;rotateTranslate)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetIdentity</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ac52ef65b6f6c22767c63cfde9041cff1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SwapColumns</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>abe926b152932e449adfc0ebca30922c5</anchor>
      <arglist>(int col1, int col2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SwapColumns3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a88caaf2b6bcf3b22aa81d6346aa1f946</anchor>
      <arglist>(int col1, int col2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SwapRows</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>abd62b252fdf1b8123e47fc7f8c66e835</anchor>
      <arglist>(int row1, int row2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SwapRows3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a2938a8b6a6972b7ba883be63bf8138ab</anchor>
      <arglist>(int row1, int row2)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetTranslatePart</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a381d092a812117df5715b68c356210ab</anchor>
      <arglist>(float tx, float ty, float tz)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetTranslatePart</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>adc44a816a7d5e2cd9796e195c7b2bba6</anchor>
      <arglist>(const float3 &amp;offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePartX</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>afc73e56f8d9be04ceedf8f65d648f366</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePartY</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a8a639428f108ed65f28b55b1cfd2f7a7</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePartZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a107ea03968548c10216452ca3f465d1d</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePart</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a4711a6efcd7123bce97fc998dbc5ff84</anchor>
      <arglist>(const float3 &amp;axisDirection, float angleRadians)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePart</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ab46ee1fe88ec0bf730b34ae66b9e2d3c</anchor>
      <arglist>(const Quat &amp;orientation)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotatePart</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a650edd88141b3240abd201b4f563bc8b</anchor>
      <arglist>(const float3x3 &amp;rotation)</arglist>
    </member>
    <member kind="function">
      <type>float4x4 &amp;</type>
      <name>operator=</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aa6252d096065495da34cee0bf98f2ef1</anchor>
      <arglist>(const float3x3 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4 &amp;</type>
      <name>operator=</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a0fd7c0aec2bc1dae80deb7f7232c86d6</anchor>
      <arglist>(const float3x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4 &amp;</type>
      <name>operator=</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a06f4bcd4de239efa4b99386696f7ebd1</anchor>
      <arglist>(const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4 &amp;</type>
      <name>operator=</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a96d1c0a89c053eb04b814ccf70993aec</anchor>
      <arglist>(const Quat &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Determinant3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a9329566d274650dbaa54b41ec9e279ac</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Determinant4</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aa1f91c2f7783acd42295edbbcb90510f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>SubMatrix</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a7aa6a80ee3a159356bdd2dbcc6ea2484</anchor>
      <arglist>(int i, int j) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Minor</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a2ab25018adf3b5aa4307ef2dcc28b3b7</anchor>
      <arglist>(int i, int j) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>Adjugate</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a2e6d08d0e748f1dc7d3c1ae5f355a6cd</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>CholeskyDecompose</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>abc44160f6efed13ca2a5eb9cbb48aa00</anchor>
      <arglist>(float4x4 &amp;outL) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>LUDecompose</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a6f422db5debc5bef5b6a88d04221a832</anchor>
      <arglist>(float4x4 &amp;outLower, float4x4 &amp;outUpper) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Inverse</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aaf1a8223fa915a0733ca306381ef3dc9</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>Inverted</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a162aa17a06cbefd828b91454bc75168f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>InverseOrthogonal</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a3b0a64febcdf69804bd73472c352e3a6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>InverseOrthogonalUniformScale</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a4e4e59e94a79c398f77b05e620591cbc</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>InverseOrthonormal</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a03419ab52fd253be017b8794690040b7</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transpose</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a9c01c70430d9fcebba73335f03e40be5</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>Transposed</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aaed05fa5f2682669cd813c1400ad9e65</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>InverseTranspose</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a5dd47ce720e2bebf6d6b77a3492475cb</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>InverseTransposed</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ac75580a150f3194d238ead2e5026f30a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Trace</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a9717f62d03f8762faa7560725a4cd5b5</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Orthogonalize3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aba9d78a267ab208f931002c74bdb5f79</anchor>
      <arglist>(int firstColumn, int secondColumn, int thirdColumn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Orthogonalize3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a431292ad28657210f29c1e6cc9aa7c49</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Orthonormalize3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ac067ee7886ab28b74e995af14c49f89f</anchor>
      <arglist>(int firstColumn, int secondColumn, int thirdColumn)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Orthonormalize3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aa1c8672a087de624ea293444100a044d</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>RemoveScale</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a374a61d0959d81b2c7dfe510f491bd22</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Pivot</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ae391f92e69b198a2358d5ce05d1cf547</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>TransformPos</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a04657e5cbe98818c38b67aef3c2ac479</anchor>
      <arglist>(const float3 &amp;pointVector) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>TransformPos</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>adc64e1f8eedfd6b1a8a2df8c6236f5cf</anchor>
      <arglist>(float x, float y, float z) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>TransformDir</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a48da53d80cce1a50986abc2cc54efda9</anchor>
      <arglist>(const float3 &amp;directionVector) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>TransformDir</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a3cf9d6a69a39f48d2d525e804603f492</anchor>
      <arglist>(float x, float y, float z) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Transform</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a9b5f656d069d61daa8963b03efd702ea</anchor>
      <arglist>(const float4 &amp;vector) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>TransformPos</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a31e902c2dd4e4418d8829da5b764c62f</anchor>
      <arglist>(float3 *pointArray, int numPoints) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>TransformPos</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a4462fae02dafaf9366a4cdfeec872077</anchor>
      <arglist>(float3 *pointArray, int numPoints, int strideBytes) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>TransformDir</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a9b996fc9bad4e0b0cd8ae5a8c8c7c283</anchor>
      <arglist>(float3 *dirArray, int numVectors) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>TransformDir</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a25d3bb03c3e52ba8014d5689e264351f</anchor>
      <arglist>(float3 *dirArray, int numVectors, int strideBytes) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>acaffb4f81c0774b1dd2b8b7187fcbe5f</anchor>
      <arglist>(float4 *vectorArray, int numVectors) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a54b2e4c8e2d22eeac93b0847a7e453ec</anchor>
      <arglist>(float4 *vectorArray, int numVectors, int strideBytes) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a64a3d89ed722d5e6b33389466d906127</anchor>
      <arglist>(const float3x3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a3e73f1c000bd74ad5a28bcd269924aba</anchor>
      <arglist>(const float3x4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ac092b9149dfe2f36507ea13e1d75598f</anchor>
      <arglist>(const float4x4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ae0121982eefa812e1bcbe3e0554315a2</anchor>
      <arglist>(const Quat &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>operator*</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ace7054778e77847818bdf9d3cf172eb0</anchor>
      <arglist>(const float4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator*</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ae40a1030dd5260d4193941d91137ab0f</anchor>
      <arglist>(float scalar) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator/</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a0738bbfa7153f2f846e34b47b09bc124</anchor>
      <arglist>(float scalar) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator+</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a7e35b6a4b893eef056044cdc9ccbc656</anchor>
      <arglist>(const float4x4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator-</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ab643fe651b4997d3a883012020f33245</anchor>
      <arglist>(const float4x4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>operator-</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a9ece53029dd81e6d242859e90bd92e7a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4x4 &amp;</type>
      <name>operator*=</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a8c89266e245d4aa195adcca70b60a31e</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>float4x4 &amp;</type>
      <name>operator/=</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a2bbe3a8922a37d36b6fa5f2b85723450</anchor>
      <arglist>(float scalar)</arglist>
    </member>
    <member kind="function">
      <type>float4x4 &amp;</type>
      <name>operator+=</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aacf09112c73a70bfdcfdff6afb5ea1bf</anchor>
      <arglist>(const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>float4x4 &amp;</type>
      <name>operator-=</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>af1951f48bb36ca2cb3dd99ab54b7d34d</anchor>
      <arglist>(const float4x4 &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFinite</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aa824a5f1c862fe7b78f7cc473ca23c71</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsIdentity</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a35d3187ec26e1ab6301cbb19ad7c5683</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsLowerTriangular</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a63ca51e1e0fe301e18311d6003528424</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsUpperTriangular</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a68551c18a99c00d75078a402abd597cd</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsInvertible</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a3f4bbc9fe383745decbeb39b71c4597a</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsSymmetric</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a76fa00400d65ebd19ddd3eb376960dc5</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsSkewSymmetric</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ad25a83131a76ab6fdec0394f5c487107</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsIdempotent</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>af979a130254d2f052ab0a12c75e9ab2c</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>HasUnitaryScale</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a614ba8c129522b407cc1c5527a9b9ee4</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>HasNegativeScale</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a288f7b03b607449fd800a8c20d9ecb12</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>HasUniformScale</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a084e390ac60bd8a74ac147d4e62c43ba</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsOrthogonal3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>abccfdd14b7508700bd6d42b5ad03b9af</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsOrthonormal3</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a803d14a61d60d6d2fed03dd97e4b66e2</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Equals</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ae7f4558efc66d99ece3f875562c9fb01</anchor>
      <arglist>(const float4x4 &amp;other, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>ContainsProjection</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ac616ff7dc7fad15fdb0cbd8b371526aa</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXYX</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a976b360e3422dcf457e3f71705ab8b0d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXZX</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a895a3ea7a880f56238621888c4ada2c2</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYXY</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a24d44604b87bfff312207f67ab427b6d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYZY</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a46b3602178d0e9e8073aa05b02a74728</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZXZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aabd0a1ca5a4def76842556a589c3dd55</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZYZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ae685f19518b5b2080beaa32e29368138</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXYZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a9668171453befd22dd31c8b880ce806b</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXZY</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a12685a71c427449f3c9736017f9d53e8</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYXZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a207f9f6d6d176ae06e457590d8da0c6f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYZX</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ab53bc47b3cd994ba0bb85852afe185cd</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZXY</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aa3b1988f17e6c7bbbde2f907c0a47a49</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZYX</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ade976d94f6dcf9e3aa3565663330cc77</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ExtractScale</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aa8bb4522554abe553f5ffd526e5075a3</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Decompose</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a6916a64a6a04a6e4d654cd4314a53b97</anchor>
      <arglist>(float3 &amp;translate, Quat &amp;rotate, float3 &amp;scale) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Decompose</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a4ded4ee43a53044f8a77a6e0e0a4d65c</anchor>
      <arglist>(float3 &amp;translate, float3x3 &amp;rotate, float3 &amp;scale) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Decompose</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a2364a9efd63255efea749561ab34e4c4</anchor>
      <arglist>(float3 &amp;translate, float3x4 &amp;rotate, float3 &amp;scale) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Decompose</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ae7cf1b162a58f3368e3b746e54922c72</anchor>
      <arglist>(float3 &amp;translate, float4x4 &amp;rotate, float3 &amp;scale) const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>float4x4</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a43936f86c1880fb8d705de422cc10cc4</anchor>
      <arglist>(const QMatrix4x4 &amp;m)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator QMatrix4x4</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aeadcf31c4c04930758a1fada27669064</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator QString</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a465ef6578747ed136b6d7b51a5110f3c</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QString</type>
      <name>toString</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ac0c175566e67e6c34ce71574a56a3a60</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QMatrix4x4</type>
      <name>ToQMatrix4x4</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a3932f6c8d44c890af9a7d383d2df53cc</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>Mul</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ac2b5f3f4772a065864b1732880e1f68a</anchor>
      <arglist>(const float3x3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>Mul</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a1439761cd18351701bd881244aae6636</anchor>
      <arglist>(const float3x4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>Mul</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a48c7df182a0b8b3335991fb46e991978</anchor>
      <arglist>(const float4x4 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>Mul</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a5b86ef7d45b1265550aab7fb16ad4aad</anchor>
      <arglist>(const Quat &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>MulPos</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aa8b75404c6d7f1ba14a999ec9335fbfa</anchor>
      <arglist>(const float3 &amp;pointVector) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>MulDir</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a671dba6df2e11a1405da9d0d9a49fd10</anchor>
      <arglist>(const float3 &amp;directionVector) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Mul</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a13ddee77049f3d4402fe05499e801d27</anchor>
      <arglist>(const float4 &amp;vector) const </arglist>
    </member>
    <member kind="function" static="yes">
      <type>static TranslateOp</type>
      <name>Translate</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ad440374d08b66382175c923c06d64dd4</anchor>
      <arglist>(float tx, float ty, float tz)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static TranslateOp</type>
      <name>Translate</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a57232e2f4161fe7a083cf02396438a25</anchor>
      <arglist>(const float3 &amp;offset)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>RotateX</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a8413beea42bb055e5ca2dbaea4e2ea5d</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>RotateX</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a018748c5f4dc2185e38484a611522bb5</anchor>
      <arglist>(float angleRadians, const float3 &amp;pointOnAxis)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>RotateY</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aa17264353bc4bdc6394526f5d09a0f1a</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>RotateY</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a55d3107df52a70c4e023353ff6242162</anchor>
      <arglist>(float angleRadians, const float3 &amp;pointOnAxis)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>RotateZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a6460abd967e0969fbf5f2150a36b1ea5</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>RotateZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a37dd282d352ef94013f4ed9c6c93ad1a</anchor>
      <arglist>(float angleRadians, const float3 &amp;pointOnAxis)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>RotateAxisAngle</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a4463288d085670d581e2f1341514c90a</anchor>
      <arglist>(const float3 &amp;axisDirection, float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>RotateAxisAngle</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>afe6559ee7144a81ea1365e2db0812f00</anchor>
      <arglist>(const float3 &amp;axisDirection, float angleRadians, const float3 &amp;pointOnAxis)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>RotateFromTo</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aee4d954172ada5e2bce0cc236ba531b4</anchor>
      <arglist>(const float3 &amp;sourceDirection, const float3 &amp;targetDirection)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>RotateFromTo</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a2797b7586164b30c2586823b88a55459</anchor>
      <arglist>(const float3 &amp;sourceDirection, const float3 &amp;targetDirection, const float3 &amp;centerPoint)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>RotateFromTo</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a1cfaf7f826274d145c1f08b58800d6bd</anchor>
      <arglist>(const float3 &amp;centerPoint, const float3 &amp;sourceDirection, const float3 &amp;targetDirection, const float3 &amp;sourceDirection2, const float3 &amp;targetDirection2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromQuat</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ad1f335c29ec10f8525e544b9c9453f9a</anchor>
      <arglist>(const Quat &amp;orientation)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromQuat</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>af9f571e70389a96a0e10c2f9f367f4bc</anchor>
      <arglist>(const Quat &amp;orientation, const float3 &amp;pointOnAxis)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromTRS</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>af7bdf8205918a84e803524d143d0a4dd</anchor>
      <arglist>(const float3 &amp;translate, const Quat &amp;rotate, const float3 &amp;scale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromTRS</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a134a8d349e0405d20791176cc8ab0e2c</anchor>
      <arglist>(const float3 &amp;translate, const float3x3 &amp;rotate, const float3 &amp;scale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromTRS</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>abc7310046aafddce39a361620f4b38d2</anchor>
      <arglist>(const float3 &amp;translate, const float3x4 &amp;rotate, const float3 &amp;scale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromTRS</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ab64627880c70c74e5f509f4e94a2e322</anchor>
      <arglist>(const float3 &amp;translate, const float4x4 &amp;rotate, const float3 &amp;scale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromEulerXYX</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a7667f6c17ce54d7c78e588b25993c363</anchor>
      <arglist>(float x2, float y, float x)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromEulerXZX</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>af1424c0e057a7ed4b0a885839a4d6255</anchor>
      <arglist>(float x2, float z, float x)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromEulerYXY</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a4c646336f3c8d7a773101c7c7d756bf9</anchor>
      <arglist>(float y2, float x, float y)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromEulerYZY</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aee38b96f4d9907284647a3d398cb4835</anchor>
      <arglist>(float y2, float z, float y)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromEulerZXZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a53e38c70db2e49a29f5189f437fdd667</anchor>
      <arglist>(float z2, float x, float z)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromEulerZYZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a2b5d78dbcfb49e57e14ee9119970656a</anchor>
      <arglist>(float z2, float y, float z)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromEulerXYZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a02f23caec720f06d01163d08eaedcf8a</anchor>
      <arglist>(float x, float y, float z)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromEulerXZY</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a971571db9da6474fcdd2a88e69f50011</anchor>
      <arglist>(float x, float z, float y)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromEulerYXZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aa53935e1a4d48e85accb2701c2f80219</anchor>
      <arglist>(float y, float x, float z)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromEulerYZX</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a8238cbdbda0a952429b1e4b338303b1d</anchor>
      <arglist>(float y, float z, float x)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromEulerZXY</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a887db3c88af775f0dfc13e64d9d01fc2</anchor>
      <arglist>(float z, float x, float y)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromEulerZYX</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a82102ca2e886a66055b9370e88f4afbe</anchor>
      <arglist>(float z, float y, float x)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static ScaleOp</type>
      <name>Scale</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a79dc6997c131c4893ce1337ef3383c9d</anchor>
      <arglist>(float sx, float sy, float sz)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static ScaleOp</type>
      <name>Scale</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a70ff67f1d19d7e245101ddffd587f988</anchor>
      <arglist>(const float3 &amp;scale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>Scale</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ac8137ce4553ef7dee007438e4327eee1</anchor>
      <arglist>(const float3 &amp;scale, const float3 &amp;scaleCenter)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>ScaleAlongAxis</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a095c8b25a68a0f28f7bb9c8a5abc0046</anchor>
      <arglist>(const float3 &amp;axis, float scalingFactor)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>ScaleAlongAxis</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>add2950edeae73ae6d778c32db423e650</anchor>
      <arglist>(const float3 &amp;axis, float scalingFactor, const float3 &amp;scaleCenter)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static ScaleOp</type>
      <name>UniformScale</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a5d870a1c2a60de4dc8a11dfe1b0b3c5f</anchor>
      <arglist>(float uniformScale)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>ShearX</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a2509cc69cfe18091591e876c498c8edb</anchor>
      <arglist>(float yFactor, float zFactor)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>ShearY</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a3ff3fb38abc50774aed3163fdb8bb5a3</anchor>
      <arglist>(float xFactor, float zFactor)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>ShearZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>af285904a82918c0aa3a85887aa6e3539</anchor>
      <arglist>(float xFactor, float yFactor)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>Reflect</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aaefc8da75772c0708acf882cb9af829e</anchor>
      <arglist>(const Plane &amp;p)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>MakePerspectiveProjection</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a70aa80926e727f923c18f7e48ea4b10a</anchor>
      <arglist>(float nearPlaneDistance, float farPlaneDistance, float horizontalFov, float verticalFov)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>MakeOrthographicProjection</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>ac462b478838bbba68ad1f87303001932</anchor>
      <arglist>(float nearPlaneDistance, float farPlaneDistance, float horizontalViewportSize, float verticalViewportSize)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>MakeOrthographicProjection</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aaaf6c63f8f6eb9f0b1904de602d83615</anchor>
      <arglist>(const Plane &amp;target)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>MakeOrthographicProjectionYZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>af3e46c4ea0a6031c8d90b7f095ba4fa8</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>MakeOrthographicProjectionXZ</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>aad09783a44f14899e80a4994d26894b0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>MakeOrthographicProjectionXY</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>abf603b66f38dfe18ca01df2fc4be5453</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>LookAt</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a7b67e3e0ae46e0a72449e8f3ed16fdf2</anchor>
      <arglist>(const float3 &amp;localForward, const float3 &amp;targetDirection, const float3 &amp;localUp, const float3 &amp;worldUp, bool rightHanded=true)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float4x4</type>
      <name>FromQMatrix4x4</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a5a91e4b32149dad8703de272cc39607e</anchor>
      <arglist>(const QMatrix4x4 &amp;m)</arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>v</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a1de0d6aa032ee9e223548e3ab71950ce</anchor>
      <arglist>[Rows][Cols]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float4x4</type>
      <name>zero</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a2ced52f5ef69ae4f99f68ccbf0737c6d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float4x4</type>
      <name>identity</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a66a5c390d2bdd552df0214fc71aa99e4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const float4x4</type>
      <name>nan</name>
      <anchorfile>classfloat4x4.html</anchorfile>
      <anchor>a7f3d89c16ca5d371809803226997c2b5</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Frustum</name>
    <filename>class_frustum.html</filename>
    <member kind="function">
      <type></type>
      <name>Frustum</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a172ae3492592e3ac891642299d628494</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>AspectRatio</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>ac4aecbe774fd6230e64c3aea2d1aaa8b</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>NearPlane</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a59408ae162b1350d444dfc429313c77c</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>FarPlane</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a75641ea65a5b4e4443f45b74bdd9c2ea</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>LeftPlane</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>af3b101d14860a2292d6f17a899d1b1ad</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>RightPlane</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a532fc26634702e85cbd0a2c970c1233e</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>TopPlane</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a52d104e8685124367f671c3aeae84fea</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>BottomPlane</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>afec48e1b59d5e2370970d5459d876d28</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>ProjectionMatrix</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a1d119e91d95f7e07e29673cadacc1860</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Ray</type>
      <name>LookAt</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>ab70204c42d666eb9961921c5cbb23bc8</anchor>
      <arglist>(float x, float y) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>NearPlanePos</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>ae190630de625f70876e9d49b49b0af5e</anchor>
      <arglist>(float x, float y) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>FarPlanePos</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a3599812dfdb531e37b16de51e372050e</anchor>
      <arglist>(float x, float y) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a78329a7d4494bf016060d9d2fec34e07</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFinite</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>ab79daadd0958310d06c846418649f031</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsDegenerate</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>ad034b1ff1b4b08e2be1b5ccf21343cb9</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>GetPlane</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>aa65502daff389d002f8d1ca8adeaac66</anchor>
      <arglist>(int faceIndex) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Volume</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a632734d828a0969a6be409683515aaff</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>RandomPointInside</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a05f59aeacd7796bfb415351ece37aaea</anchor>
      <arglist>(LCG &amp;rng) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Translate</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a142c88c132455cc170c6383598a21474</anchor>
      <arglist>(const float3 &amp;offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Scale</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>ac1ec7b4d62d8d7213197900c11b2400a</anchor>
      <arglist>(const float3 &amp;centerPoint, float uniformScaleFactor)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Scale</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>abccbd5c7cc0a79e52beaae5a315287aa</anchor>
      <arglist>(const float3 &amp;centerPoint, const float3 &amp;nonuniformScaleFactors)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a58f77713a9b61795327708a78d256d77</anchor>
      <arglist>(const float3x3 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a64d6c84a9243c5176947783524735154</anchor>
      <arglist>(const float3x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a69729c5ee0e480047db78c11da4af585</anchor>
      <arglist>(const float4x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a7d6b162862abff1dd44825794e351d79</anchor>
      <arglist>(const Quat &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>GetPlanes</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>afbdeb77390493e7730b27a0015ad3527</anchor>
      <arglist>(Plane *outArray) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>GetCornerPoints</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a2ab2da130b96af7798a1052c6daccb40</anchor>
      <arglist>(float3 *outPointArray) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>CornerPoint</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>abd3d13f45aa982335c90b16fe37d2226</anchor>
      <arglist>(int cornerIndex) const </arglist>
    </member>
    <member kind="function">
      <type>AABB</type>
      <name>ToAABB</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>ab0ef6440c15997ae2b9f242b1cb10ec3</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>OBB</type>
      <name>ToOBB</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a0ddce36587c89f460f559ab33d522868</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a957bc5ed604bd76e803d66f0495e7390</anchor>
      <arglist>(const Ray &amp;ray, float &amp;outDistance) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>aaf66d6be014782f264d8bd0ee887f114</anchor>
      <arglist>(const Line &amp;line, float &amp;outDistance) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a547318253a78ea15aa66725dbd812ef8</anchor>
      <arglist>(const LineSegment &amp;lineSegment, float &amp;outDistance) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>aedd1dabe80bd311fc69867309750976c</anchor>
      <arglist>(const AABB &amp;aabb) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a8d71d5107cc54db625b9b44668063646</anchor>
      <arglist>(const OBB &amp;obb) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a162ff467b5c89f13619a6da80d527810</anchor>
      <arglist>(const Plane &amp;plane) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>aeaa52b85fd2000cd00607267b9d428f8</anchor>
      <arglist>(const Sphere &amp;sphere) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a4d614d236ab70ae0e0a0451b0d3330d1</anchor>
      <arglist>(const Ellipsoid &amp;ellipsoid) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>ab3a4beedc8c6b0e5376befc05a2d1224</anchor>
      <arglist>(const Triangle &amp;triangle) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a9ab0a9520cbf1a986ccb294756d978f4</anchor>
      <arglist>(const Cylinder &amp;cylinder) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a7f2a5298d7650016d2dd82a738380c3b</anchor>
      <arglist>(const Torus &amp;torus) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a8d6f32c95a8611d6e6bf7513698a1bed</anchor>
      <arglist>(const Frustum &amp;frustum) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>afb2461df71729e5a4ee76b1a593773a3</anchor>
      <arglist>(const Polyhedron &amp;polyhedron) const </arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>pos</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a72db383c0bcd8794cdf7a82272df8b68</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>front</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>abcc27c2a92b3d6ec5b3d6eea1b9e75e5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>up</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>ad96ead1bd926f40e2e58ff5457e165c1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>nearPlaneDistance</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>afa5a9f8bfbffb09bce9655ed9330f2dd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>farPlaneDistance</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a50274f825eab085e70f74d5910de41b5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>horizontalFov</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>a85f0f34973996b1b21993c3aaa4e9759</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>verticalFov</name>
      <anchorfile>class_frustum.html</anchorfile>
      <anchor>afd73965ea57a3bd50b801395f8b798d0</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>HitInfo</name>
    <filename>struct_hit_info.html</filename>
    <member kind="enumeration">
      <name>HitResult</name>
      <anchorfile>struct_hit_info.html</anchorfile>
      <anchor>a713af3479fb6dab1c10c06798c6f2661</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>NoHit</name>
      <anchorfile>struct_hit_info.html</anchorfile>
      <anchor>a713af3479fb6dab1c10c06798c6f2661a9f53c97934f78e386e13c71f944bcbac</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>Intersect</name>
      <anchorfile>struct_hit_info.html</anchorfile>
      <anchor>a713af3479fb6dab1c10c06798c6f2661ae5b18c9724717fdf1a8125c9e7643502</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>AInsideB</name>
      <anchorfile>struct_hit_info.html</anchorfile>
      <anchor>a713af3479fb6dab1c10c06798c6f2661a9b6a1e7c52fde05ca56d9ce0b17e9021</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>BInsideA</name>
      <anchorfile>struct_hit_info.html</anchorfile>
      <anchor>a713af3479fb6dab1c10c06798c6f2661a0797dae037e185e85dbfb97346633574</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>HitResult</type>
      <name>result</name>
      <anchorfile>struct_hit_info.html</anchorfile>
      <anchor>a739963028cbd707aa2864484f27f3589</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>point</name>
      <anchorfile>struct_hit_info.html</anchorfile>
      <anchor>ab19323e82624c7b8e3b4f2b7eafe47da</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>normalA</name>
      <anchorfile>struct_hit_info.html</anchorfile>
      <anchor>acef5916a011d600dc0e07aa1c678840c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>normalB</name>
      <anchorfile>struct_hit_info.html</anchorfile>
      <anchor>a0ea9b307a884650fa521ac759b104a5f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>LCG</name>
    <filename>class_l_c_g.html</filename>
    <member kind="function">
      <type></type>
      <name>LCG</name>
      <anchorfile>class_l_c_g.html</anchorfile>
      <anchor>a889667620f66d4623d05b2dee7eefbad</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>LCG</name>
      <anchorfile>class_l_c_g.html</anchorfile>
      <anchor>abacf8c63581e1b1f09367c0de7d7b719</anchor>
      <arglist>(u32 seed, u32 multiplier=69621, u32 increment=0, u32 modulus=0x7FFFFFFF)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Seed</name>
      <anchorfile>class_l_c_g.html</anchorfile>
      <anchor>a82ad5e210c71c5ba3e78735701934707</anchor>
      <arglist>(u32 seed, u32 multiplier=69621, u32 increment=0, u32 modulus=0x7FFFFFFF)</arglist>
    </member>
    <member kind="function">
      <type>u32</type>
      <name>Int</name>
      <anchorfile>class_l_c_g.html</anchorfile>
      <anchor>af3dcfd49de301682208e2fe79ad0f896</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>u32</type>
      <name>MaxInt</name>
      <anchorfile>class_l_c_g.html</anchorfile>
      <anchor>a37a253791cad7c61d0308c36e5128055</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>u32</type>
      <name>IntFast</name>
      <anchorfile>class_l_c_g.html</anchorfile>
      <anchor>a3d5219d664623c71bb87a4891c4b92bf</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Int</name>
      <anchorfile>class_l_c_g.html</anchorfile>
      <anchor>a966927d3637777696902be1886707ae4</anchor>
      <arglist>(int a, int b)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Float</name>
      <anchorfile>class_l_c_g.html</anchorfile>
      <anchor>a37ed8813214617be0ee1453340f52dba</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Float</name>
      <anchorfile>class_l_c_g.html</anchorfile>
      <anchor>ab5f4c39679330cba5a033bf0a72bd760</anchor>
      <arglist>(float a, float b)</arglist>
    </member>
    <member kind="variable" protection="private">
      <type>u32</type>
      <name>multiplier</name>
      <anchorfile>class_l_c_g.html</anchorfile>
      <anchor>ab12bfeb839f1e6bf6f286c292a705bf1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" protection="private">
      <type>u32</type>
      <name>increment</name>
      <anchorfile>class_l_c_g.html</anchorfile>
      <anchor>abcb3864c13b9d24337d96a0f0cc7c22d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" protection="private">
      <type>u32</type>
      <name>modulus</name>
      <anchorfile>class_l_c_g.html</anchorfile>
      <anchor>af8b96cf65067cdf745b7ad37bfb82ff1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" protection="private">
      <type>u32</type>
      <name>lastNumber</name>
      <anchorfile>class_l_c_g.html</anchorfile>
      <anchor>a843e29516d2d38d52da07851ac2e0172</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Line</name>
    <filename>class_line.html</filename>
    <member kind="function">
      <type></type>
      <name>Line</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>acc11b8a429d8cdd63ba6803dff5602b3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Line</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a80e5b2d59b4fcad8e0149779287e9c06</anchor>
      <arglist>(const float3 &amp;pos, const float3 &amp;dir)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Line</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a41f08eb43b15bd7b65a58db5362d9310</anchor>
      <arglist>(const Ray &amp;ray)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Line</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>afcaf8e7803eaedeee6a999841534b017</anchor>
      <arglist>(const LineSegment &amp;lineSegment)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>GetPoint</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>af63ffc85019ec47ce50beafe84a7abaa</anchor>
      <arglist>(float distance) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>aeda4125cd567e4a8a23541f00934b37a</anchor>
      <arglist>(const float3x3 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a831e6418e7c2963466f8ace484bb46a7</anchor>
      <arglist>(const float3x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>af26b90cb24393f667973d4c87f649cf7</anchor>
      <arglist>(const float4x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>ae8caddcfb254ae398686be2793b35a30</anchor>
      <arglist>(const Quat &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a423e8c14b6d43ca975cf1913cfce4d5f</anchor>
      <arglist>(const float3 &amp;point, float distanceThreshold=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>abe922db9fe263c1b45f8db2af8442e3b</anchor>
      <arglist>(const float3 &amp;point, float *d=0) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a41026370b68368352631e05b3c3a452a</anchor>
      <arglist>(const Ray &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>acfd7b9941b3805c544f3c74dbd054156</anchor>
      <arglist>(const Line &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a23eb7280423785bb1f219376b5dd63c9</anchor>
      <arglist>(const LineSegment &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>abd7fb324590c1a464069034f71c22ba1</anchor>
      <arglist>(const Triangle &amp;triangle, float *d, float3 *intersectionPoint) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>acee7220416d6529dbc44a912591944ff</anchor>
      <arglist>(const Plane &amp;plane, float *d) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a2c667dc5ce358f1cfc1216a472a09542</anchor>
      <arglist>(const Sphere &amp;s, float3 *intersectionPoint=0, float3 *intersectionNormal=0, float *d=0) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a552dd1a96141d3a56730599963a965de</anchor>
      <arglist>(const AABB &amp;aabb, float *dNear, float *dFar) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a26680a3288b699038070b75ece93bab6</anchor>
      <arglist>(const OBB &amp;obb, float *dNear, float *dFar) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a22fc0b560a4f6d09dca2383887454835</anchor>
      <arglist>(const float3 &amp;targetPoint, float *d=0) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a7b974669c1784412c38d936b6cdc29fa</anchor>
      <arglist>(const Ray &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a1acf53483319b4c1dee80c0de7bfda5e</anchor>
      <arglist>(const Line &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>acdfe789f64bb7ff16cc4ceb9ffb42e69</anchor>
      <arglist>(const LineSegment &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>Ray</type>
      <name>ToRay</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>ab4cf1931cedd7cb5efbcf208383861d5</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>LineSegment</type>
      <name>ToLineSegment</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a8a54d4aa3c8ac7711e983768e9c22414</anchor>
      <arglist>(float d) const </arglist>
    </member>
    <member kind="function" static="yes">
      <type>static bool</type>
      <name>AreCollinear</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>aa9fa89ac5d6d7db41c3ed3db33e2116f</anchor>
      <arglist>(const float3 &amp;p1, const float3 &amp;p2, const float3 &amp;p3, float epsilon=1e-3f)</arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>pos</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a624f6d0b9fea3d45eec2ba06d26f15e6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>dir</name>
      <anchorfile>class_line.html</anchorfile>
      <anchor>a8632f1adae2b74f7387ab24c967f0d56</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>LineSegment</name>
    <filename>class_line_segment.html</filename>
    <member kind="function">
      <type></type>
      <name>LineSegment</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a4c2da2ff2d3979f28bfcb24948a8e6a9</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>LineSegment</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a27ac7f6acc05193fe36dc40e3e2acf05</anchor>
      <arglist>(const float3 &amp;a, const float3 &amp;b)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>LineSegment</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a560d69fcfdf2621bedd892dbcaf8e806</anchor>
      <arglist>(const Ray &amp;ray, float d)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>LineSegment</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>af4dbb8f0f38f901baabb669f7704651c</anchor>
      <arglist>(const Line &amp;line, float d)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>GetPoint</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>ab7e85346b607129b9c7643c41face40b</anchor>
      <arglist>(float d) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>CenterPoint</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>adda2bbb6503f83e18b86c8b81ce0330e</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Reverse</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a969ca61a6cf5ea0d7315d17bfe53f45c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Dir</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a570cdc497a771665091f2a500c46f0c2</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>abba0ffbb0184c2c7c9da4b3b2cde4085</anchor>
      <arglist>(const float3x3 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a0749e0e3aec314c143010a0cd9cbe97e</anchor>
      <arglist>(const float3x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a7338016480f67938acb21b21707d2a00</anchor>
      <arglist>(const float4x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a37a88cf1bbc2f5cb21c5b7546fc74674</anchor>
      <arglist>(const Quat &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Length</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a9e12f256d8086f2bf20913dc89d5418a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>LengthSq</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a8a9b34967aa741a5b2a3b6b8b206c550</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a15119cc408e83571d9b92ec30867e234</anchor>
      <arglist>(const float3 &amp;point, float distanceThreshold=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>aff79a5d7276d0fc8acbf4d5f30cd61e7</anchor>
      <arglist>(const float3 &amp;point, float *d=0) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a97a0550e4ea461b4319dc7d3021f1434</anchor>
      <arglist>(const Ray &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a1002522b0b970f0b088d474a51b2cfd1</anchor>
      <arglist>(const Line &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a538152dabc893b9768a2ad4848cae038</anchor>
      <arglist>(const LineSegment &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>ae2062bbc9cd51523fefa76e2c0c813b2</anchor>
      <arglist>(const float3 &amp;point, float *d=0) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>ae4f23659205dd7a28f7e2a6fb69687f2</anchor>
      <arglist>(const Ray &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a157d3c718f83462f35e214606000873b</anchor>
      <arglist>(const Line &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a02d291923d0f207fc550cc3abbb183b1</anchor>
      <arglist>(const LineSegment &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a6ba68e9cf950ed16e6d354c4f1c47311</anchor>
      <arglist>(const Plane &amp;plane) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a1f914d8a2f73e00ef90057d130c84b98</anchor>
      <arglist>(const Triangle &amp;triangle, float *d, float3 *intersectionPoint) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>adce951d8092d74a3e8fd1d51266ef79d</anchor>
      <arglist>(const Sphere &amp;s, float3 *intersectionPoint=0, float3 *intersectionNormal=0, float *d=0) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a28adc94b780ce40051ac95a06ae859b4</anchor>
      <arglist>(const AABB &amp;aabb, float *dNear=0, float *dFar=0) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a8d1d90dc38f0c143c60773fbde06ac76</anchor>
      <arglist>(const OBB &amp;obb, float *dNear, float *dFar) const </arglist>
    </member>
    <member kind="function">
      <type>Ray</type>
      <name>ToRay</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>ad5f8892e677f14e0f216ad3d7a20ae88</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Line</type>
      <name>ToLine</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a3b2ef3d4c89a3043aa9f2982d767cb9a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>a</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>aebe15b3975c9000ebc7fc15db20a50be</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>b</name>
      <anchorfile>class_line_segment.html</anchorfile>
      <anchor>a24d611255b252e26983ff4eb3caf7757</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>LSBT</name>
    <filename>class_l_s_b_t.html</filename>
    <templarg>Bits</templarg>
    <member kind="enumvalue">
      <name>val</name>
      <anchorfile>class_l_s_b_t.html</anchorfile>
      <anchor>a36dff2d302cf06c14968127e1014adeea5d6d4a884ba5f65b06177d8b5a3ad4be</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>MatrixProxy</name>
    <filename>class_matrix_proxy.html</filename>
    <templarg>Cols</templarg>
    <member kind="function">
      <type>CONST_WIN32 float</type>
      <name>operator[]</name>
      <anchorfile>class_matrix_proxy.html</anchorfile>
      <anchor>aeb4191e5f25b84b68c2e8e1eb153808d</anchor>
      <arglist>(int col) const </arglist>
    </member>
    <member kind="function">
      <type>float &amp;</type>
      <name>operator[]</name>
      <anchorfile>class_matrix_proxy.html</anchorfile>
      <anchor>a21ba265acc36635e89ef4ee4a2eae273</anchor>
      <arglist>(int col)</arglist>
    </member>
    <member kind="variable" protection="private">
      <type>float</type>
      <name>v</name>
      <anchorfile>class_matrix_proxy.html</anchorfile>
      <anchor>a90abb7f11eacaf66a97fe8fd0a51a67f</anchor>
      <arglist>[Cols]</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>OBB</name>
    <filename>class_o_b_b.html</filename>
    <member kind="function">
      <type></type>
      <name>OBB</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>af752af068c6fc1787f06556b6a71c929</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>OBB</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>ad991c58a1f319ce9925799aa4a247838</anchor>
      <arglist>(const AABB &amp;aabb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetNegativeInfinity</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>aecbf4afa2fcc9a3091cd6a1bbf4d628f</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFrom</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a2aa6647a2f602bf1c4a45d63ad7ef5e8</anchor>
      <arglist>(const AABB &amp;aabb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFrom</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>addbee6bb28475d9f2c8cb66ea1bf3ef0</anchor>
      <arglist>(const AABB &amp;aabb, const float3x3 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFrom</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a33dc896b80c97148d367e0b4fabe4f11</anchor>
      <arglist>(const AABB &amp;aabb, const float3x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFrom</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>af22966b0e6d30caa17768244e89c8450</anchor>
      <arglist>(const AABB &amp;aabb, const float4x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFrom</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>ac56536d1511ed7dc0a3b8607ac2a08d0</anchor>
      <arglist>(const AABB &amp;aabb, const Quat &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFrom</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a889704f53dae8e19841949de700f8624</anchor>
      <arglist>(const Sphere &amp;sphere)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFromApproximate</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>ac49f88f2d2fa3206c36f560ece2f1f11</anchor>
      <arglist>(const float3 *pointArray, int numPoints)</arglist>
    </member>
    <member kind="function">
      <type>AABB</type>
      <name>MinimalEnclosingAABB</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>aaaecb1733903f781f85eb87470b8cb6f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>AABB</type>
      <name>MaximalContainedAABB</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a08b5ef00f42a8a7e333b0eee899e588f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Size</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>ae7baee462a47158fc82d3769118b66cd</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>HalfSize</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>acedd48ed82395dd5359dab64d30df4c0</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Diagonal</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a0742192e315ee39238d5118ae191b6ce</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>HalfDiagonal</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a079a648e36c78365f39fcb47c9e905a6</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>WorldToLocal</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a99ba9985c706b5503e45bc9eb56038f5</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>LocalToWorld</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a5eaf352d70ac77331dfc54a1a002c024</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Sphere</type>
      <name>MinimalEnclosingSphere</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a2935814ca04038f1b4e40ebe1843857a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Sphere</type>
      <name>MaximalContainedSphere</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>aa20c2530c65ba655a7bc26fd3b78dc56</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFinite</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a2d5c51250812a0644a8288bb97f8dd7f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsDegenerate</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a0233e258331e142806c16df0dcc9e46d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>CenterPoint</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a67bae9a765725df7dcbfc23e923b3cca</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>PointInside</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a870be5fb5bd1776877c2d8cafbc32056</anchor>
      <arglist>(float x, float y, float z) const </arglist>
    </member>
    <member kind="function">
      <type>LineSegment</type>
      <name>Edge</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a0c7fffbbd9b199a8499fd4aa2c7186fe</anchor>
      <arglist>(int edgeIndex) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>CornerPoint</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a046c7c064277ffc22ade8dd39c310f7e</anchor>
      <arglist>(int cornerIndex) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>PointOnEdge</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>aac9304a5c55cc135d85067f70ce25049</anchor>
      <arglist>(int edgeIndex, float u) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>FaceCenterPoint</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>ad0ca3a300362fbf7485095a763b0495c</anchor>
      <arglist>(int faceIndex) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>FacePoint</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a1b56dededb024a7ec743fd93d223c836</anchor>
      <arglist>(int faceIndex, float u, float v) const </arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>FacePlane</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a4e57cff43db8700dff9b22d84c341c03</anchor>
      <arglist>(int faceIndex) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>GetCornerPoints</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a2cb54729a3a716a3735563be5700269d</anchor>
      <arglist>(float3 *outPointArray) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>GetFacePlanes</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a613401b402550d582f5c683856578c3b</anchor>
      <arglist>(Plane *outPlaneArray) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Volume</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a255749b8901e3d32667e743cd824192d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>SurfaceArea</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a3dfd183556acadce38a0bf44e43f9808</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>RandomPointInside</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>abef43c6c8c212a964b83e1d0f61c759d</anchor>
      <arglist>(LCG &amp;rng) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>RandomPointOnSurface</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a9294dd7a26c280e2dacfbec255a2ad99</anchor>
      <arglist>(LCG &amp;rng) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>RandomPointOnEdge</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>ad9a3982d8a42398da5d0a634d07b576b</anchor>
      <arglist>(LCG &amp;rng) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>RandomCornerPoint</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>af6cd1dcc77daa88d0d02bd048ef57a85</anchor>
      <arglist>(LCG &amp;rng) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Translate</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a75e64bae1caa36e5ac2c803a3713f3f6</anchor>
      <arglist>(const float3 &amp;offset)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Scale</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>abf13f865e8023d8cc10d14329f76d4e4</anchor>
      <arglist>(const float3 &amp;centerPoint, float scaleFactor)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Scale</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>ae5f38962f84f7f586a0ec7fe6afe86c8</anchor>
      <arglist>(const float3 &amp;centerPoint, const float3 &amp;scaleFactor)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a2a27c38835431cd0d7b01bf87f1380b6</anchor>
      <arglist>(const float3x3 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>aebd9837c7c947f22f3895ba7fcddceca</anchor>
      <arglist>(const float3x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a5d521556fbcda3903bcccff85c31191f</anchor>
      <arglist>(const float4x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a9d3e1a030419cf82aa1811a2b0f2927a</anchor>
      <arglist>(const Quat &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>ae2910650cd07805827b884efb6f55f58</anchor>
      <arglist>(const float3 &amp;targetPoint) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a84f674caec0591372655917658fbcf76</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>ab004df688976f806742554d8ec863b8b</anchor>
      <arglist>(const Ray &amp;ray, float3 *outClosestPoint, float *outClosestDistance) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a33d5200af7c32dfc6fb273bf8e2a870d</anchor>
      <arglist>(const Line &amp;line, float3 *outClosestPoint, float *outClosestdistance) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a165d22763e16268af895bf833bd24d7f</anchor>
      <arglist>(const LineSegment &amp;lineSegment, float3 *outClosestPoint, float *outClosestDistance) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>af7c1a205adfefc40875405dbd6fec002</anchor>
      <arglist>(const AABB &amp;aabb, float3 *outClosestPoint, float3 *outClosestPointOther) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>af3b675e4fbbe00f6eb8811c7d6a76c66</anchor>
      <arglist>(const OBB &amp;obb, float3 *outClosestPoint, float3 *outClosestPointOther) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a88fc5af13a1810c17670bee7f2e4e2ee</anchor>
      <arglist>(const Plane &amp;plane, float3 *outClosestPoint, float3 *outClosestPointOther) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>ac49fd0ba52f831e50b1e826a98103185</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a514940c3a43f3c7884941668845b5614</anchor>
      <arglist>(const LineSegment &amp;lineSegment) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a414ad7269ee775ac7bba527fd98e76c2</anchor>
      <arglist>(const AABB &amp;aabb) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a058b385625f1f5d93f02a58373863d7c</anchor>
      <arglist>(const OBB &amp;obb) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a45ec2b2d4e66a8fbfcb34ebd07c645ba</anchor>
      <arglist>(const AABB &amp;aabb) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>af458aeef3b9ea354cd37092c1d09f097</anchor>
      <arglist>(const OBB &amp;obb, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>aa4b447b167dd874d4f290e1307c79d85</anchor>
      <arglist>(const Plane &amp;plane) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a44e04768b9552bce4c550fa3eda3e023</anchor>
      <arglist>(const Ray &amp;ray, float *dNear, float *dFar) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>ab70c38417f550bf4064c63bb7c1f0d39</anchor>
      <arglist>(const Line &amp;line, float *dNear, float *dFar) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>ae34c57e96671a430ad1610df01fda294</anchor>
      <arglist>(const LineSegment &amp;lineSegment, float *dNear, float *dFar) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>ad13925fe6a72c58489fdc801a0556763</anchor>
      <arglist>(const Sphere &amp;sphere, float3 *closestPointOnOBB) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a6dd3ccb96e105a5ef485382b0132f8a9</anchor>
      <arglist>(const Triangle &amp;triangle) const </arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>ExtremePointsAlongDirection</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a8381406034c203f0bac2d9b3bebf4fa3</anchor>
      <arglist>(const float3 &amp;dir, const float3 *pointArray, int numPoints, int *idxSmallest, int *idxLargest)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static OBB</type>
      <name>PCAEnclosingOBB</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a62188ca47a97467ab2ba874a32b1de44</anchor>
      <arglist>(const float3 *pointArray, int numPoints)</arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>pos</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>aee2fce54d439ab6fadb53eb74f977512</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>r</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>a8e4c58c2b0cdf4d1b3000424ea5b961b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>axis</name>
      <anchorfile>class_o_b_b.html</anchorfile>
      <anchor>aad7dab57cc5afa62797b3dd2db56e505</anchor>
      <arglist>[3]</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Plane</name>
    <filename>class_plane.html</filename>
    <member kind="function">
      <type></type>
      <name>Plane</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>acac0d9c003e0ab10d07b146c3566a0c7</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Plane</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a9039e9eafc806fd6f35b04cea51291e8</anchor>
      <arglist>(const float3 &amp;normal, float d)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Plane</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a03a90ab2eb95a9191fdfcabdb3f55b19</anchor>
      <arglist>(const float3 &amp;v1, const float3 &amp;v2, const float3 &amp;v3)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Plane</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>ac5ab5f7445d3cce2efa57b8c77a04c5a</anchor>
      <arglist>(const float3 &amp;point, const float3 &amp;normal)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Plane</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a9a035c5e744050a74f37d761c8339e6a</anchor>
      <arglist>(const Ray &amp;ray, const float3 &amp;normal)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Plane</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>ade4de571f12b30c4d45395cd83e7a17f</anchor>
      <arglist>(const Line &amp;line, const float3 &amp;normal)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Plane</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a7154ae1e6083f1ee8b47a4b3698e3024</anchor>
      <arglist>(const LineSegment &amp;lineSegment, const float3 &amp;normal)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a899157b05437e4a54450ec91131f5fe4</anchor>
      <arglist>(const float3 &amp;v1, const float3 &amp;v2, const float3 &amp;v3)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a996838903bc03cac3f95d5a1c3850e81</anchor>
      <arglist>(const float3 &amp;point, const float3 &amp;normal)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>PointOnPlane</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a27988f92535772d752bc680904d90e0d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>ab5892d0a9448fa76a945be0f852ec58d</anchor>
      <arglist>(const float3x3 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a400a48da0a54a4fd5dc7e6a4a5f111a5</anchor>
      <arglist>(const float3x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a09a28fa391c0d4d4581c06970c7d6128</anchor>
      <arglist>(const float4x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a278ae49cd2c57b9ecc999c3b49dc70d2</anchor>
      <arglist>(const Quat &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsInPositiveDirection</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a08b4fd7ce0d4fdca7888089fae5ef0b0</anchor>
      <arglist>(const float3 &amp;directionVector) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsOnPositiveSide</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a42df718381e096eea62ed2c883007b91</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>AreOnSameSide</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a7f134f450008da7e57b6314378158287</anchor>
      <arglist>(const float3 &amp;p1, const float3 &amp;p2) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>ac4aff58afe44065b4e23e9fb7790284f</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>SignedDistance</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a751afe2ca7dad4bb7887e712cdb7eabf</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>OrthoProjection</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a1137046902f85bce7faf342625f9b181</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>ObliqueProjection</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a65e7d0e6553eb8fef0ac3de4b83d7064</anchor>
      <arglist>(const float3 &amp;obliqueProjectionDir) const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>ReflectionMatrix</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a793c9b80a073c0ab5739fc99ecfcffe3</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Reflect</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a05e731bb98b9f3c6f3d5965186ea597f</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Refract</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a7ce23f3a7a2ae9b767a15061d114960c</anchor>
      <arglist>(const float3 &amp;normal, float negativeSideRefractionIndex, float positiveSideRefractionIndex) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Project</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a1b6fd62e305b3b356becf2d0a1a926be</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a6ba06ef2eae130111c3e2aff542fcd37</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ObliqueProject</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>ad3e433d0f457cf4e82dce41be6d148e9</anchor>
      <arglist>(const float3 &amp;point, const float3 &amp;obliqueProjectionDir) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a2b17b0bed81564143caff9e50f5672c3</anchor>
      <arglist>(const float3 &amp;point, float distanceThreshold=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a7745fac65c95c605e43615b59c17b8d9</anchor>
      <arglist>(const Plane &amp;plane, Line *outLine=0) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a2947bfc9617005e2d4bb7c34c9e9f12f</anchor>
      <arglist>(const Plane &amp;plane, const Plane &amp;plane2, Line *outLine=0, float3 *outPoint=0) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a4836a02279d471a937e0d8dfebde2f74</anchor>
      <arglist>(const Ray &amp;ray, float *d) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>ad8ecbdc17e635efacdf51e56407bce51</anchor>
      <arglist>(const Line &amp;line, float *d) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a239690b2422edf1b2a2fa5d350eddec3</anchor>
      <arglist>(const LineSegment &amp;lineSegment, float *d) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a7db6b3e4a7728ba4cc0e850313ad1cd3</anchor>
      <arglist>(const Sphere &amp;sphere) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>ab77e392a837c97a5b8a12f36c225726d</anchor>
      <arglist>(const AABB &amp;aabb) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a412f5ed25eb557b18d223074db410fb6</anchor>
      <arglist>(const OBB &amp;obb) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>ae765b4140dcd5d2bebb91877d2369307</anchor>
      <arglist>(const Triangle &amp;triangle) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a5d51180c8191a6742ec5f07d686263f7</anchor>
      <arglist>(const Frustum &amp;frustum) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Clip</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a0a0a78b3473480aa20c590c27f61b1e5</anchor>
      <arglist>(float3 &amp;a, float3 &amp;b) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Clip</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a05f6f36c795c8150aecc4fb73926622b</anchor>
      <arglist>(LineSegment &amp;line) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Clip</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a09c87c63cd961a6586744ec7525cd120</anchor>
      <arglist>(const Line &amp;line, Ray &amp;outRay) const </arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Clip</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a748ac958ee114afca2125c3bb28af40b</anchor>
      <arglist>(const Triangle &amp;triangle, Triangle &amp;t1, Triangle &amp;t2) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsParallel</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>ab2004d16c775ee2a298353c78c241a34</anchor>
      <arglist>(const Plane &amp;plane, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>DihedralAngle</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a153ade007820d532288c45e7921cfcc4</anchor>
      <arglist>(const Plane &amp;plane) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Equals</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a63c3d37fa113165b1b211957f5a4bfc3</anchor>
      <arglist>(const Plane &amp;other, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>Circle</type>
      <name>GenerateCircle</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a548e859b37d9bf67e70e2b1c342f9fa2</anchor>
      <arglist>(const float3 &amp;circleCenter, float radius) const </arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>normal</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a8b7aa1989587f0ec88c265f9716e456e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>d</name>
      <anchorfile>class_plane.html</anchorfile>
      <anchor>a61fc789fce8fbe72914f5397f1bbed44</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Polygon</name>
    <filename>class_polygon.html</filename>
    <member kind="function">
      <type></type>
      <name>Polygon</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>ac183e712f8be1e13f1c9d5b4d4512ead</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>NumEdges</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>a4866a4a0e56e152ac708571496617600</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>LineSegment</type>
      <name>Edge</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>aa0d45162eb0b03dd8f99dc620e08d84c</anchor>
      <arglist>(int i) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>DiagonalExists</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>af1f089066020913df422ca3f267bb52d</anchor>
      <arglist>(int i, int j) const </arglist>
    </member>
    <member kind="function">
      <type>LineSegment</type>
      <name>Diagonal</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>a433fbd95750977ef334967205f2ed1c4</anchor>
      <arglist>(int i, int j) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsConvex</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>a9559b31604517fbc960834af8752aedc</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsPlanar</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>a386c1a21d4d6e2274a585e197b360dbb</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsSimple</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>adb451af840c80dfa25222c265f6ba681</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>GetPlane</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>ac64c3a0f977dcc8b50f503e9f760519d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsSelfIntersecting</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>a424b433bd2c38d2d2f4d17a7cbf3b38b</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProjectToPlane</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>ae0bce2387dc598f5cd764926343ba85c</anchor>
      <arglist>(const Plane &amp;plane)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsSelfIntersecting</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>aad5c2d7c4717d635d4200c512acbd476</anchor>
      <arglist>(const float3 &amp;viewDirection) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>a6adce735bb910bf172857efb5c5ab040</anchor>
      <arglist>(const float3 &amp;point, const float3 &amp;viewDirection) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Area</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>a98d867eda56423605e6a038e3a245b72</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Perimeter</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>a02e751fb39b77d0ebd434c980449e67f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Centroid</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>ad17a887fbf10f5a1e81ecd36846c1284</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsConcaveVertex</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>ac1b2ef1b61cfa62b168e89f5dc4a958d</anchor>
      <arglist>(int i) const </arglist>
    </member>
    <member kind="function">
      <type>Polygon</type>
      <name>ConvexHull</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>a166b25066e62b0149710a6acd09e79f5</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsSupportingPoint</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>aa2a8f340642d0aba36fee14887f587b8</anchor>
      <arglist>(int i) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsSupportingPoint</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>a8ab009eeda5e9f4ee551001956b4e11c</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function" static="yes">
      <type>static bool</type>
      <name>IsConvexQuad</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>a338ed57ae0940921e620d222153af61b</anchor>
      <arglist>(const float3 &amp;pointA, const float3 &amp;pointB, const float3 &amp;pointC, const float3 &amp;pointD)</arglist>
    </member>
    <member kind="variable">
      <type>std::vector&lt; float3 &gt;</type>
      <name>points</name>
      <anchorfile>class_polygon.html</anchorfile>
      <anchor>a9be79951ef1308194d049bdd5ce0a17b</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Polynomial</name>
    <filename>class_polynomial.html</filename>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>SolveQuadratic</name>
      <anchorfile>class_polynomial.html</anchorfile>
      <anchor>a6ff98f554777aa927f539f164c629fa3</anchor>
      <arglist>(float a, float b, float c, float &amp;root1, float &amp;root2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>SolveCubic</name>
      <anchorfile>class_polynomial.html</anchorfile>
      <anchor>afb3e810392fb96020ef8dac85d58c69d</anchor>
      <arglist>(float a, float b, float c, float d, float &amp;root1, float &amp;root2, float &amp;root3)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int</type>
      <name>SolveQuartic</name>
      <anchorfile>class_polynomial.html</anchorfile>
      <anchor>a7314daecfec4942c8ecda6749839998e</anchor>
      <arglist>(float a, float b, float c, float d, float &amp;root1, float &amp;root2, float &amp;root3, float &amp;root4)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>PowIntT</name>
    <filename>class_pow_int_t.html</filename>
    <templarg>Base</templarg>
    <templarg>Power</templarg>
    <member kind="enumvalue">
      <name>val</name>
      <anchorfile>class_pow_int_t.html</anchorfile>
      <anchor>a522bcdc996132278bc5929d6fc55844da2c944c455d993901c840a3dc7ca4cf22</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>val</name>
      <anchorfile>class_pow_int_t.html</anchorfile>
      <anchor>a522bcdc996132278bc5929d6fc55844da2c944c455d993901c840a3dc7ca4cf22</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Quat</name>
    <filename>class_quat.html</filename>
    <member kind="function">
      <type></type>
      <name>Quat</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>afe432f2aac598c876a51b16dce29aa13</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Quat</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>aee21b25254dd0d4638a9def7ca86d44e</anchor>
      <arglist>(const Quat &amp;rhs)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Quat</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>afe12e3c30d24bb480db899adbf7a922c</anchor>
      <arglist>(const float *data)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Quat</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a538923d790309d926cfa4c22b60a99be</anchor>
      <arglist>(const float3x3 &amp;rotationMatrix)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Quat</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>aeaf94761a22836e7eef0350c813a50ac</anchor>
      <arglist>(const float3x4 &amp;rotationMatrix)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Quat</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a368b2e3b0b3f0695c70ef8c26b108a34</anchor>
      <arglist>(const float4x4 &amp;rotationMatrix)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Quat</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>aee6d96df262aa9a4057687150487335f</anchor>
      <arglist>(float x, float y, float z, float w)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Quat</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a6ef8e718caef9d6f758ff2c5c29f323e</anchor>
      <arglist>(const float3 &amp;rotationAxis, float rotationAngleRadians)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>WorldX</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ad58cea56fccddd465c9632c720ef11c8</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>WorldY</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a3e87586f47584a3d307f748789875bf8</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>WorldZ</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a51138979389e1fb8e854af0adaba7c26</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Axis</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a520f79d3e9fc32bad3cbc26c1866b357</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Angle</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>afbda0e5f4adc66231e0eae6e94294953</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Dot</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a6b24d6d69c7c74568c020a84c196b135</anchor>
      <arglist>(const Quat &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>LengthSq</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a777aa9745bcf77aa838ad10a1efccd3b</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Length</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>abd807aaa6f64a63fe8d67f22dfe160aa</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Normalize</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a85a480602cedfd6543ac428db8ad2d14</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>Quat</type>
      <name>Normalized</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a21892cd4d1788963154dbb37f9184e67</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsNormalized</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a048a0e91e83a78e069ac84793cccf1dd</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsInvertible</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a32d8c5f2e20f84cab13ae0d2d9c80cd0</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFinite</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a9c130317897d679f8bffa295ff0e8a30</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Equals</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>aca001c1c735a3e969402a1c1c8073d26</anchor>
      <arglist>(const Quat &amp;rhs, float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>float *</type>
      <name>ptr</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ad1ec8c3cb121bbee9d4e5a7082d328a1</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>const float *</type>
      <name>ptr</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a53ab9abdbf511b9f195ada8e8bfeb4a7</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Inverse</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a1bd4058a0acac5a54ebb05d51bc89710</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>Quat</type>
      <name>Inverted</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>abcbbef9f8c398a5306804d2f21217a8b</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>InverseAndNormalize</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>acdac9573f74d736a5da90aba36168765</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Conjugate</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>adddd4f2e270b3987534339b2d0fd688a</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>Quat</type>
      <name>Conjugated</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a205824848ad12cb10fcb8c6c06c50af7</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Transform</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>afc6d8d40e16934c00f4e783f8fc24f2e</anchor>
      <arglist>(float x, float y, float z) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Transform</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a4777d1213b7f19420acab540f8ecad04</anchor>
      <arglist>(const float3 &amp;vec) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Transform</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>aa5a27b54248ee691236d59a3d6771ff2</anchor>
      <arglist>(const float4 &amp;vec) const </arglist>
    </member>
    <member kind="function">
      <type>Quat</type>
      <name>Lerp</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ac5dcff83678a90c1c1693f325ea1e463</anchor>
      <arglist>(const Quat &amp;target, float t) const </arglist>
    </member>
    <member kind="function">
      <type>Quat</type>
      <name>Slerp</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>aa63fddf35c3e057ab27a56243df80792</anchor>
      <arglist>(const Quat &amp;target, float t) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>AngleBetween</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a0cf7e6775bab35047166c3c29581542f</anchor>
      <arglist>(const Quat &amp;target) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>AxisFromTo</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ad0525458bb779a740ae458665315f699</anchor>
      <arglist>(const Quat &amp;target) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ToAxisAngle</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a22e86b4766c1b9d8d8c8c9c53ec85e72</anchor>
      <arglist>(float3 &amp;rotationAxis, float &amp;rotationAngleRadians) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetFromAxisAngle</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>aeedcfb4b771a06d132dccb5832b08e67</anchor>
      <arglist>(const float3 &amp;rotationAxis, float rotationAngleRadians)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a94d576fcd7bb7d1e85f79e8513c6d160</anchor>
      <arglist>(const float3x3 &amp;matrix)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a7f5d35eea94d145d273f7985463589ae</anchor>
      <arglist>(const float3x4 &amp;matrix)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a522d43d1e5f3d87d67723bd2782a7ab0</anchor>
      <arglist>(const float4x4 &amp;matrix)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Set</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>abca8ac0233d2d5f2ba3c97dde35ca696</anchor>
      <arglist>(float x, float y, float z, float w)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>LookAt</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>af4e734a6af2f83b6bfaa794115b29f71</anchor>
      <arglist>(const float3 &amp;localForward, const float3 &amp;targetDirection, const float3 &amp;localUp, const float3 &amp;worldUp)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXYX</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a3ac1f39a0683f284d9ae1a06fa4832a4</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXZX</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a426f692381b6068fcfcd6af9aed3bbfa</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYXY</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a888a08575ab522afef5e653cb3b13069</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYZY</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a5245d6d73369557d525f7632320aa89a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZXZ</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a62bf836fe408dab67f6c51feae4749cf</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZYZ</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ae0fe18fcfb54a48ed543e2a3f5f13c58</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXYZ</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ad1c4d694a32caded333e31a24e9d9f3e</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerXZY</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a657692f31e3fac72b1b7ba575f84ab59</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYXZ</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>afd4cabddd65fb245eb292bbfd8114c13</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerYZX</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a878d3b6839b97bfe045fcb0275921fc0</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZXY</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>aac95d976f0da7ec68a922abfc22286e1</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ToEulerZYX</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>afa6bb6769a02b68e21d15881ffe66316</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>ToFloat3x3</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a704e400131ef6d7c6faace8e5a794241</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>ToFloat3x4</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a8c0b9f7c8453158b3bc02eb94c3256bb</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>ToFloat4x4</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>acc172c52c46dd1a8017489cd3b523e4e</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Quat</type>
      <name>operator*</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a667de8cc5d16a89e6ed7295b2776abbc</anchor>
      <arglist>(const Quat &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>operator*</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a8ffaa448dc8f3e7bed8b947f3993998e</anchor>
      <arglist>(const float3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>Quat</type>
      <name>operator/</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a2fb917f4fa7cffc0be76fadc395f30ac</anchor>
      <arglist>(const Quat &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Quat</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>abdbf011f63d05bf0ed938809594f012b</anchor>
      <arglist>(const QQuaternion &amp;other)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator QQuaternion</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ae14835b90f13b33bae2528b52eeabd6c</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator QString</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ae5051d8496264e3fca1b5ae3b99ce171</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QString</type>
      <name>toString</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a036e9f125e5d8d2b05db0596374d4e31</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>QQuaternion</type>
      <name>ToQQuaternion</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>aea307120f6bdeb9b2ec5d684b045e5b4</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Quat</type>
      <name>Mul</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a0dca7d9dac5c9da53d2c4f638108b27b</anchor>
      <arglist>(const Quat &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>Quat</type>
      <name>Mul</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>aba602043c5351aeb1a7c2a26ced4456b</anchor>
      <arglist>(const float3x3 &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Mul</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a8b7511266f778b48f8afaf4e5be68adb</anchor>
      <arglist>(const float3 &amp;vector) const </arglist>
    </member>
    <member kind="function">
      <type>float4</type>
      <name>Mul</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a44f775014db56fd6c2498b5f6b7e723b</anchor>
      <arglist>(const float4 &amp;vector) const </arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>Lerp</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a8f2e2d31b1b12fff65b8d515d15dbdc2</anchor>
      <arglist>(const Quat &amp;source, const Quat &amp;target, float t)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>Slerp</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a844f56ec5305ddff2453b797b94ca355</anchor>
      <arglist>(const Quat &amp;source, const Quat &amp;target, float t)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>RotateX</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a0e4c80860506466c4b0c1370d1bae581</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>RotateY</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a26f12042e280d1f0cd61dc0187ecafb3</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>RotateZ</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a553e0cfd1e114106e2c165c241b15b34</anchor>
      <arglist>(float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>RotateAxisAngle</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ae38b901de671870af2ebc6f240094661</anchor>
      <arglist>(const float3 &amp;axisDirection, float angleRadians)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>RotateFromTo</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>abd90363830eb8ab9ddcb3aff4352eeac</anchor>
      <arglist>(const float3 &amp;sourceDirection, const float3 &amp;targetDirection)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>RotateFromTo</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>af7007214c529a0060f55c506027bee49</anchor>
      <arglist>(const float3 &amp;sourceDirection, const float3 &amp;targetDirection, const float3 &amp;sourceDirection2, const float3 &amp;targetDirection2)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromEulerXYX</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a9509aae4b45450bd583fea673e83445a</anchor>
      <arglist>(float x2, float y, float x)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromEulerXZX</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>af7e61e10176e936781894bb746556680</anchor>
      <arglist>(float x2, float z, float x)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromEulerYXY</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a78b32ddee4fc0d1945879ea6a08a7d9b</anchor>
      <arglist>(float y2, float x, float y)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromEulerYZY</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>abb46dffe428ba7f8eafc44932e3c54a4</anchor>
      <arglist>(float y2, float z, float y)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromEulerZXZ</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a4820334da6f1270e67101f2f7e70c5cd</anchor>
      <arglist>(float z2, float x, float z)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromEulerZYZ</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a92cb4f1dc174ac241787f6730ba5cbd0</anchor>
      <arglist>(float z2, float y, float z)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromEulerXYZ</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ac36126451dd918e00dd3594e3b79b60f</anchor>
      <arglist>(float x, float y, float z)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromEulerXZY</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>aa6b4bbd9e46eb9a17361213df4241ca4</anchor>
      <arglist>(float x, float z, float y)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromEulerYXZ</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a2989fc94494d0dfaa412e3328dd8266b</anchor>
      <arglist>(float y, float x, float z)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromEulerYZX</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>acf8254f298b48eb3bcb91307b5ecdd05</anchor>
      <arglist>(float y, float z, float x)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromEulerZXY</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ad5e5524ff0a813a8387009e38622382e</anchor>
      <arglist>(float z, float x, float y)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromEulerZYX</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>abd8b6c2b86d7a510ce9935a4926fc224</anchor>
      <arglist>(float z, float y, float x)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromString</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a59224a7f05205361b41e66e389840266</anchor>
      <arglist>(const char *str)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromQQuaternion</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>adb7361cb176a856f817fe28ff822fbbe</anchor>
      <arglist>(const QQuaternion &amp;q)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Quat</type>
      <name>FromString</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a9822952121690766c72e12c2886d524c</anchor>
      <arglist>(const QString &amp;str)</arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>x</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a15db053bd6750a53e33bf30a0250c288</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>y</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ac6045ee4149a978a4ce429c132e46fa2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>z</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a611fbb79f7b802da4f1c65a31c2c5fea</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>w</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ad3aeee2134443ec0897793834209b0c0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const Quat</type>
      <name>identity</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ad86f5f7cfef6582f26537209a0441e12</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const Quat</type>
      <name>nan</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a39a1a9bacd05ae7c12a1840ad00025cc</anchor>
      <arglist></arglist>
    </member>
    <member kind="function" protection="private">
      <type>Quat</type>
      <name>operator*</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a82b64e3282e84cda04f05db06bd4f962</anchor>
      <arglist>(float scalar) const </arglist>
    </member>
    <member kind="function" protection="private">
      <type>Quat</type>
      <name>operator/</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a6c5d901289ae6633210a67fec60d7333</anchor>
      <arglist>(float scalar) const </arglist>
    </member>
    <member kind="function" protection="private">
      <type>Quat</type>
      <name>operator+</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a5949c59ffd10abf894f113ba53d62f38</anchor>
      <arglist>(const Quat &amp;rhs) const </arglist>
    </member>
    <member kind="function" protection="private">
      <type>Quat</type>
      <name>operator-</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>ab3a9109cd4527c66c2ead2fdc8533301</anchor>
      <arglist>(const Quat &amp;rhs) const </arglist>
    </member>
    <member kind="function" protection="private">
      <type>Quat</type>
      <name>operator-</name>
      <anchorfile>class_quat.html</anchorfile>
      <anchor>a01739a8b02cd7279dfe83a836bf058ec</anchor>
      <arglist>() const </arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Ray</name>
    <filename>class_ray.html</filename>
    <member kind="function">
      <type></type>
      <name>Ray</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a2e3d2c29f2df4ab3da10da79d4acb852</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Ray</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a6d03009cd3a6fa3c92cfba9bb1678558</anchor>
      <arglist>(const float3 &amp;pos, const float3 &amp;dir)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Ray</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a51a12d30825907bf7a7f1fd80e5fce70</anchor>
      <arglist>(const Line &amp;line)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Ray</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a005e3c7d50917fe188662ca7034f8f64</anchor>
      <arglist>(const LineSegment &amp;lineSegment)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>GetPoint</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a81ca5d12b91229f4a518558b54437126</anchor>
      <arglist>(float distance) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>ad87a8d115bb679496404507ea3396c92</anchor>
      <arglist>(const float3x3 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a84ea505f5885f7b610a94737c65078d8</anchor>
      <arglist>(const float3x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a41e125a4fbcbf07b672b11b9f8b13347</anchor>
      <arglist>(const float4x4 &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Transform</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a20dc67429f10765f1ccfd862e8ef6784</anchor>
      <arglist>(const Quat &amp;transform)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>acb8f0d1c74157c76813bac06992df68f</anchor>
      <arglist>(const float3 &amp;point, float distanceThreshold=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>ab76b09a6bfa55298501059ebe38a3186</anchor>
      <arglist>(const float3 &amp;point, float *d=0) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>aa7a9c1858781a26b882418fbbe2ee03b</anchor>
      <arglist>(const Ray &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a0447fda528bcadac187c175e36bdeaef</anchor>
      <arglist>(const Line &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>aedb7ab29bae0e6fda165b3062fdc7009</anchor>
      <arglist>(const LineSegment &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a5736d568a9b5380af2aaa7ec7bb5feb5</anchor>
      <arglist>(const float3 &amp;targetPoint, float *d=0) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a053e6d63f1688d61d5b1168d0b6e6380</anchor>
      <arglist>(const Ray &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a84f7f9bc6357dbbed72ededf96dec646</anchor>
      <arglist>(const Line &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a7c6e34651192b14db8a849570f0be002</anchor>
      <arglist>(const LineSegment &amp;other, float *d=0, float *d2=0) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a351e3042c2ca2b21e71eba7fa80fed24</anchor>
      <arglist>(const Triangle &amp;triangle, float *d, float3 *intersectionPoint) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a62b2e37ae6006e4e9a5c59d9f50195c2</anchor>
      <arglist>(const Plane &amp;plane, float *d) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a61a0928b1217bc4f52955eae76955b03</anchor>
      <arglist>(const Sphere &amp;s, float3 *intersectionPoint=0, float3 *intersectionNormal=0, float *d=0) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a9ae03805415131d691258aa551adc97b</anchor>
      <arglist>(const AABB &amp;aabb, float *dNear, float *dFar) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a4f9e5aafeebeec8185b2671d38661156</anchor>
      <arglist>(const OBB &amp;obb, float *dNear, float *dFar) const </arglist>
    </member>
    <member kind="function">
      <type>Line</type>
      <name>ToLine</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a8f2b0002cc010399519a8e0199f22844</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>LineSegment</type>
      <name>ToLineSegment</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a0452715e438cc6cd494d87f492c9d193</anchor>
      <arglist>(float d) const </arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>pos</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>a9f3d1739df0ef482f670b1408d11329b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>dir</name>
      <anchorfile>class_ray.html</anchorfile>
      <anchor>adb316c7a3d9ec022ca0a75ded7a88611</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Rect</name>
    <filename>class_rect.html</filename>
    <member kind="function">
      <type></type>
      <name>Rect</name>
      <anchorfile>class_rect.html</anchorfile>
      <anchor>a911e531b86de33734dd7de3456722115</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Rect</name>
      <anchorfile>class_rect.html</anchorfile>
      <anchor>a07bda43ddfcc2a3587f6f466ecb6e5ee</anchor>
      <arglist>(int left_, int top_, int width, int height)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Width</name>
      <anchorfile>class_rect.html</anchorfile>
      <anchor>ae220661d52f3439c31cdd3a412432520</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>Height</name>
      <anchorfile>class_rect.html</anchorfile>
      <anchor>a0ef39dd5bbe303fc06c4418c1a4839d3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>left</name>
      <anchorfile>class_rect.html</anchorfile>
      <anchor>ab0841a51d249d8560ddca36054fd1e57</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>top</name>
      <anchorfile>class_rect.html</anchorfile>
      <anchor>abb300f9f75fcb27d2a27ae5e260d2ed2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>right</name>
      <anchorfile>class_rect.html</anchorfile>
      <anchor>aedfb34dad0634ee71105ddfd2c885243</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>bottom</name>
      <anchorfile>class_rect.html</anchorfile>
      <anchor>a34f470ffa9b5f83ad3be7764ebd403b4</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>ScaleOp</name>
    <filename>class_scale_op.html</filename>
    <member kind="function">
      <type></type>
      <name>ScaleOp</name>
      <anchorfile>class_scale_op.html</anchorfile>
      <anchor>ae2f5a1f5a207ff4402d474b75593e615</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>ScaleOp</name>
      <anchorfile>class_scale_op.html</anchorfile>
      <anchor>a496a35ef27dea0f982a994036dfea4ae</anchor>
      <arglist>(const float3 &amp;scale)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>ScaleOp</name>
      <anchorfile>class_scale_op.html</anchorfile>
      <anchor>ad92672ceb9c194e32d87c663bd71d870</anchor>
      <arglist>(float sx, float sy, float sz)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Offset</name>
      <anchorfile>class_scale_op.html</anchorfile>
      <anchor>a4ef78d08c7847217878a3178e7b4841f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator float3x3</name>
      <anchorfile>class_scale_op.html</anchorfile>
      <anchor>ac36b77f21d97f833e10ee1439a1cc53d</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator float3x4</name>
      <anchorfile>class_scale_op.html</anchorfile>
      <anchor>a1b5d1e721c64a5f827745238be617b89</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator float4x4</name>
      <anchorfile>class_scale_op.html</anchorfile>
      <anchor>af63a190465c06c91f6126c489f044d2a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>ToFloat3x3</name>
      <anchorfile>class_scale_op.html</anchorfile>
      <anchor>a1fcf0a5f88c9887470bb5e410fbfc23f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>ToFloat3x4</name>
      <anchorfile>class_scale_op.html</anchorfile>
      <anchor>a7bbf47249f8ed5f2c322d197b09d5ff3</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>ToFloat4x4</name>
      <anchorfile>class_scale_op.html</anchorfile>
      <anchor>abe853ca2d356460ff8c07bdeecb5b8bc</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>x</name>
      <anchorfile>class_scale_op.html</anchorfile>
      <anchor>afa11cb23584ea17857d81a0e7f5e433f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>y</name>
      <anchorfile>class_scale_op.html</anchorfile>
      <anchor>ab7d208eea9b429c886fb008863f3e732</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>z</name>
      <anchorfile>class_scale_op.html</anchorfile>
      <anchor>a26a4ab33855cb3e26a99e7e06c9bd966</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>SortByPolarAngle</name>
    <filename>class_sort_by_polar_angle.html</filename>
    <member kind="function">
      <type>bool</type>
      <name>operator()</name>
      <anchorfile>class_sort_by_polar_angle.html</anchorfile>
      <anchor>ab1bd17857bfefedbe2607f14cf7ad66a</anchor>
      <arglist>(const float2 &amp;a, const float2 &amp;b) const </arglist>
    </member>
    <member kind="variable">
      <type>float2</type>
      <name>perspective</name>
      <anchorfile>class_sort_by_polar_angle.html</anchorfile>
      <anchor>a92ab51fa073ba4e35fd5f365b14d5e77</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Sphere</name>
    <filename>class_sphere.html</filename>
    <member kind="function">
      <type></type>
      <name>Sphere</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a890a63ff583cb88e7ec4e840b4ef5eb9</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Sphere</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>ac64071cefd1dec29eeeea8850cf999f7</anchor>
      <arglist>(const float3 &amp;center, float radius)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Sphere</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a565d1107f9ea776d0741127f5e979676</anchor>
      <arglist>(const float3 &amp;pointA, const float3 &amp;pointB)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Sphere</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>ab9af8f03e2d7cafb08eabb4adb319231</anchor>
      <arglist>(const float3 &amp;pointA, const float3 &amp;pointB, const float3 &amp;pointC)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Sphere</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a86a627114be7447064f98fd06ae5dd1a</anchor>
      <arglist>(const float3 &amp;pointA, const float3 &amp;pointB, const float3 &amp;pointC, const float3 &amp;pointD)</arglist>
    </member>
    <member kind="function">
      <type>AABB</type>
      <name>MinimalEnclosingAABB</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>acbfdcd52a00947530f0a339bebad5294</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>AABB</type>
      <name>MaximalContainedAABB</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a441f596873aa5a8a63da6b36189d6fdc</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetNegativeInfinity</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a241326e6bac1fb2e0d3047000108b7fb</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Volume</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>af96dd641abb8d9650e258f0cf104240a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>SurfaceArea</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a6f280acfa43636a4860c869ca161c1f3</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsFinite</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a21dc5af6fc556cab6f9e0f13d80b7c33</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsDegenerate</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a366bc6e6cb876f679e4b1a88c8359462</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>ab7d8ab2c9fab5e97e10fc614c7409b4a</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Distance</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a1a9f263ec2c2adc16a8fbc9e2d66129d</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>abaa807e5be4b3187130b7a00e94e2f6a</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>abe5de83a3e85fde0ec2a8ca7ca4695ac</anchor>
      <arglist>(const LineSegment &amp;l, float3 *intersectionPoint=0, float3 *intersectionNormal=0, float *d=0) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>af4ddd7b727e288d1425c875738b2f30c</anchor>
      <arglist>(const Line &amp;l, float3 *intersectionPoint=0, float3 *intersectionNormal=0, float *d=0) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a9e1b065ef893095613479450a01647a7</anchor>
      <arglist>(const Ray &amp;r, float3 *intersectionPoint=0, float3 *intersectionNormal=0, float *d=0) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>adab21c2ab9b202a579cfb896b5255de5</anchor>
      <arglist>(const Plane &amp;plane) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>af41483606cacf7dc8f57fce60c1126ea</anchor>
      <arglist>(const AABB &amp;aabb, float3 *closestPointOnAABB) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a2ca0bf43dab593edb184def9bdb4451f</anchor>
      <arglist>(const OBB &amp;obb, float3 *closestPointOnOBB) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>ab1dca1214e516461872bea21fd4f7457</anchor>
      <arglist>(const Triangle &amp;triangle, float3 *closestPointOnTriangle) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>ac1824fb7c67a863cbed80d7d62b2103d</anchor>
      <arglist>(const Sphere &amp;sphere) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Enclose</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>ab54c21fd4edabceeb00784ef08a6e36f</anchor>
      <arglist>(const AABB &amp;aabb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Enclose</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a55f1bd16d4d046f2e47c842cd64e8da1</anchor>
      <arglist>(const OBB &amp;obb)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Enclose</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>ad44abce637881980d7b41c3f28d468dd</anchor>
      <arglist>(const Sphere &amp;sphere)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Enclose</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a3dcc1af9958dbaefaaa0a8ac21a8cfe1</anchor>
      <arglist>(const LineSegment &amp;lineSegment)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Enclose</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>ac78242191dda26582c2c230a3cba5124</anchor>
      <arglist>(const float3 &amp;point)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Enclose</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>aded2706b2ac39045e899f6095e6ed111</anchor>
      <arglist>(const float3 *pointArray, int numPoints)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static Sphere</type>
      <name>FastEnclosingSphere</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a0ddd9b8db5bb63ef7eb0bd9ad125be10</anchor>
      <arglist>(const float3 *pointArray, int numPoints)</arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>pos</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>af81384e5e37e9089a9aa731f429d4592</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>r</name>
      <anchorfile>class_sphere.html</anchorfile>
      <anchor>a2b1b412591a00eb30cb012e444f7192d</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Transform</name>
    <filename>class_transform.html</filename>
    <member kind="function">
      <type></type>
      <name>Transform</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>aa08ca4266efabc768973cdeea51945ab</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Transform</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a63bf74598d65963a1c96a641a139ac35</anchor>
      <arglist>(const float3 &amp;pos_, const float3 &amp;rot_, const float3 &amp;scale)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Transform</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a88020d9a0a9ee2688e0bcee53110cbb7</anchor>
      <arglist>(const float3x3 &amp;m)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Transform</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a1eaef766e5818aa9425af747868a7bf6</anchor>
      <arglist>(const float3x4 &amp;m)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Transform</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a1a6726818c7166782815c7854c317d2f</anchor>
      <arglist>(const float4x4 &amp;m)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetPos</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a5408cd315607ca0b669ba589541ce63e</anchor>
      <arglist>(const float3 &amp;v)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetPos</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a937e4fe70fa57e9fb9ec01b44265a303</anchor>
      <arglist>(float x, float y, float z)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotation</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a80e28cb7ca8818e944c6177ed6ca7bfe</anchor>
      <arglist>(float x, float y, float z)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetScale</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a3f4051624bff3c6d6c5be9fd9433fe2f</anchor>
      <arglist>(float x, float y, float z)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetScale</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>ac261c1ab9b2d4a2c990dc3b3d2daa873</anchor>
      <arglist>(const float3 &amp;s)</arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>ToFloat3x4</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a47e8aa7a576b0d628a20cc923320b415</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>ToFloat4x4</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>ab9194d72c49cb4214769edfbde2d7925</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FromFloat3x4</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>adbaabf5694d75dfbbeb9a814355037c2</anchor>
      <arglist>(const float3x4 &amp;m)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>FromFloat4x4</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a07222990c8ae4ab69a6ec895e59225d6</anchor>
      <arglist>(const float4x4 &amp;m)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetRotationAndScale</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>afdf88637d088ff969db66909728a744f</anchor>
      <arglist>(const float3x3 &amp;mat)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetOrientation</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a5f3c045db46970e7f555d12fa5c1c4c5</anchor>
      <arglist>(const float3x3 &amp;mat)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>SetOrientation</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a9a1c6f2f87f6d2fde9a5b6aecbfafe53</anchor>
      <arglist>(const Quat &amp;q)</arglist>
    </member>
    <member kind="function">
      <type>float3x3</type>
      <name>Orientation3x3</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>adfb558be921a73dab9606cdd442d81e7</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Quat</type>
      <name>Orientation</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a0fd2c41b16828501de55c183ff1a384a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Transform</type>
      <name>operator*</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>aac212b2c917e1fd9ce3c7a93ff633658</anchor>
      <arglist>(const Transform &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>Transform</type>
      <name>Mul</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a116f90b038bcf09bfe782434b155c7be</anchor>
      <arglist>(const Transform &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator float3x4</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a7aa470429c91c58bc982bde30649395c</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator float4x4</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>accd7585e9d7697624e93d0a6c71ce99f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>ad7a14c049f0deef8af2b0520a3465b9a</anchor>
      <arglist>(const Transform &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a50fc366392d282da233613e44abd5b3e</anchor>
      <arglist>(const Transform &amp;rhs) const </arglist>
    </member>
    <member kind="function">
      <type>QString</type>
      <name>toString</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a28bfa24a7b5083bb121e9651c9ab2bee</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>pos</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a94775b579f4163babedc5616470e6f8e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>rot</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a5bc4c290347608493cf102d13f3696d6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>scale</name>
      <anchorfile>class_transform.html</anchorfile>
      <anchor>a9577e939313c3a35f53a3b84ac77a2ce</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>TranslateOp</name>
    <filename>class_translate_op.html</filename>
    <member kind="function">
      <type></type>
      <name>TranslateOp</name>
      <anchorfile>class_translate_op.html</anchorfile>
      <anchor>a0204795c029a30b785c3cc303d7453ac</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>TranslateOp</name>
      <anchorfile>class_translate_op.html</anchorfile>
      <anchor>a0e954a3bd9371e441d7e58eb6a997263</anchor>
      <arglist>(const float3 &amp;offset)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>TranslateOp</name>
      <anchorfile>class_translate_op.html</anchorfile>
      <anchor>a6e1ae5375f1e1012ebeee95c7060ef15</anchor>
      <arglist>(float x, float y, float z)</arglist>
    </member>
    <member kind="function">
      <type>float3x4</type>
      <name>ToFloat3x4</name>
      <anchorfile>class_translate_op.html</anchorfile>
      <anchor>adb462671cabc76140b85d3495eb3e400</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float4x4</type>
      <name>ToFloat4x4</name>
      <anchorfile>class_translate_op.html</anchorfile>
      <anchor>a13d2bda791376cf6bed1077f6f70f5a4</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Offset</name>
      <anchorfile>class_translate_op.html</anchorfile>
      <anchor>a6cf0d8f70f8bb149708b541c2f60facd</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator float3x4</name>
      <anchorfile>class_translate_op.html</anchorfile>
      <anchor>a51a1f772398bf4473bf82c83ba99913f</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator float4x4</name>
      <anchorfile>class_translate_op.html</anchorfile>
      <anchor>a5c4bc760c3aa3c2e4b66bea1277a7f03</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>x</name>
      <anchorfile>class_translate_op.html</anchorfile>
      <anchor>a87ca4fbc13ae4adf5eeced2b60611f1c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>y</name>
      <anchorfile>class_translate_op.html</anchorfile>
      <anchor>af5526f92b8f923bf1f170ee23b57cade</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float</type>
      <name>z</name>
      <anchorfile>class_translate_op.html</anchorfile>
      <anchor>ac2fa48941b375e05d159feab19086906</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>Triangle</name>
    <filename>class_triangle.html</filename>
    <member kind="function">
      <type></type>
      <name>Triangle</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>aaefe4ed500c07918d30c6f0e286332c5</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>Triangle</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a4dc0397e156cab591c2fb6b6c51cd7d3</anchor>
      <arglist>(const float3 &amp;a, const float3 &amp;b, const float3 &amp;c)</arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Barycentric</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a686e5b36503b6af7983cab49f0bb6594</anchor>
      <arglist>(const float3 &amp;point) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Point</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a2b29f756cf1149594c57c61018109043</anchor>
      <arglist>(float u, float v, float w) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Point</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a2fb8ea2441da0523c08714ce01871812</anchor>
      <arglist>(float u, float v) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Point</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>ae5fa98c0f9fa03ca8ae30202d6cfcdaa</anchor>
      <arglist>(const float3 &amp;barycentric) const </arglist>
    </member>
    <member kind="function">
      <type>float</type>
      <name>Area</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>ad3885c40e78c33327a622351dbb07ed7</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>Plane</type>
      <name>GetPlane</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a79326fefb2cb7ce14b9ed6aba8d80cc9</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>Normal</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a2ad1a779108f28c4f412797edd5828b7</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>UnnormalizedNormal</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>ae7723232d584cf972b7002642506305a</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>IsDegenerate</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>ad11c4d89a46c96bf36ee8eeb2fa7a4ed</anchor>
      <arglist>(float epsilon=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Contains</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a9b29acd69889f03786445e704ad1f622</anchor>
      <arglist>(const float3 &amp;point, float triangleThickness=1e-3f) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Distance</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a59e3c83f4901499a410defb171aa0f26</anchor>
      <arglist>(const float3 &amp;point)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a4e4ac89137c05a9ddd4736c7c1314e93</anchor>
      <arglist>(const LineSegment &amp;lineSegment, float *d, float3 *intersectionPoint) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>aae9a4a3356a2a84c167f9b916008a390</anchor>
      <arglist>(const Line &amp;line, float *d, float3 *intersectionPoint) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a1f7892ec24d5b51f729c8bbc0fd20aa3</anchor>
      <arglist>(const Ray &amp;ray, float *d, float3 *intersectionPoint) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>ae73ce87d3d7c0ff05708bd17b1be0f85</anchor>
      <arglist>(const Plane &amp;plane) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a29829a7eab3da30199e4ed314e639934</anchor>
      <arglist>(const Sphere &amp;sphere, float3 *closestPointOnTriangle) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a028885262e327bcb18c2d75c4c2382cb</anchor>
      <arglist>(const Triangle &amp;triangle, LineSegment *outLine) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a960e339bdc32b4f99c7bb5dc65c7c746</anchor>
      <arglist>(const AABB &amp;aabb) const </arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>Intersects</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a2a742585f3e4adf5540e5cb3980b00e8</anchor>
      <arglist>(const OBB &amp;obb) const </arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ProjectToAxis</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a0f925df1d5f648fcd30eda680b3ea30e</anchor>
      <arglist>(const float3 &amp;axis, float &amp;dMin, float &amp;dMax) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a096d6ab71390035b7059d5c11062bc6d</anchor>
      <arglist>(const float3 &amp;targetPoint) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>aa27a86014d6421ca41402421dfdd2e6d</anchor>
      <arglist>(const LineSegment &amp;other, float3 *otherPt) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>aae4440034bd1702b87552d0731711052</anchor>
      <arglist>(const Ray &amp;other, float3 *otherPt) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>ab9a4ba0bea34bf4682bb9bab11fd3714</anchor>
      <arglist>(const Line &amp;other, float3 *otherPt) const </arglist>
    </member>
    <member kind="function">
      <type>float3</type>
      <name>ClosestPoint</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a06f42609d4d5bdec6844ad0273177073</anchor>
      <arglist>(const Triangle &amp;other, float3 *otherPt) const </arglist>
    </member>
    <member kind="function" static="yes">
      <type>static bool</type>
      <name>BarycentricInsideTriangle</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a841663c26d102cdb7051b7ec21ba784f</anchor>
      <arglist>(const float3 &amp;barycentric)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float</type>
      <name>Area2D</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a755f2926a64f5ce5c09960c7b48742d6</anchor>
      <arglist>(const float2 &amp;p1, const float2 &amp;p2, const float2 &amp;p3)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static float</type>
      <name>SignedArea</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a9fb56471e79b893d6e4669ba077f6b33</anchor>
      <arglist>(const float3 &amp;point, const float3 &amp;a, const float3 &amp;b, const float3 &amp;c)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static bool</type>
      <name>IsDegenerate</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a6c96633c0fa46486f02c8a618e269552</anchor>
      <arglist>(const float3 &amp;p1, const float3 &amp;p2, const float3 &amp;p3, float epsilon=1e-3f)</arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>a</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>a60032e205c77bf7ea872af591926ef15</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>b</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>ad10fe2087ce2c2aea57063c83281fe22</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>float3</type>
      <name>c</name>
      <anchorfile>class_triangle.html</anchorfile>
      <anchor>aac57f92363f5c03d76ec2622eee90898</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="namespace">
    <name>math</name>
    <filename>namespacemath.html</filename>
    <class kind="class">math::FixedPoint</class>
    <member kind="function">
      <type>void</type>
      <name>Add</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a18a80f0cf2d7f8cd781cdf4015ef94e9</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;dst, const FixedPoint&lt; T, F2 &gt; &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Add</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>af1388b77882562b5ed1835e9ee314697</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;dst, const T &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Sub</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a832aad38f4d991280acb98fafcaa2898</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;dst, const FixedPoint&lt; T, F2 &gt; &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Sub</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a6cc3abe80fc18a9bbf9135ee433a5ea6</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;dst, const T &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Mul</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a46d7e0bf17d2480593ba5b3d2d69c0b7</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;dst, const T &amp;src)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>MulExtraFast</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a2ba7973d9dc3ff9679781f6df90c136f</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F2 &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>MulFast</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a10ff077aa8525420c0e9af9209acfd61</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>MulPrecise</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>aa8641fd334cbbaf6ccb14e3c1d0e5a11</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>Div</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>aa735426ee1efa28cfa30c2fabf2121e7</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>DivExtraFast</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>aa19dc8641b25c14eb0e71b957ce6e644</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F2 &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt; &amp;</type>
      <name>operator+=</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a42107c70ddbbb8c61c184aee29cefdc9</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt; &amp;</type>
      <name>operator+=</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a369125fd6f6205362b876e66e09b6c10</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt;</type>
      <name>operator+</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a83cd3d0bff199a8fd7c53e8af03b2c68</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt;</type>
      <name>operator+</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>ad235c328e404e97537bc020f0f7f60bd</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt; &amp;</type>
      <name>operator-=</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>ae189df4785fd4bd7142cabd3731f961e</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt;</type>
      <name>operator-</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a3fe6d21dc03913ab3dca23d5d252caea</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt; &amp;</type>
      <name>operator*=</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a04e98020c5614269ada88fe8bbb16c1e</anchor>
      <arglist>(FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>const FixedPoint&lt; T, F &gt;</type>
      <name>operator*</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a85dd63be8a2fcbdd798ce028c01c5d83</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator&lt;</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>aea412d4cd40008d66ba9d03bad694f7e</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator&lt;</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>ab1a184f7d5631523691ec837133e9ae3</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>ad58d1aca0c00a465c8cb8c4f8814f317</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const T &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator==</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>abf4beebbcb47a69d81889c19ed114548</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>operator!=</name>
      <anchorfile>namespacemath.html</anchorfile>
      <anchor>a46de8314a7608e056a9c69dc4d208c7c</anchor>
      <arglist>(const FixedPoint&lt; T, F &gt; &amp;a, const FixedPoint&lt; T, F &gt; &amp;b)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>math::FixedPoint</name>
    <filename>classmath_1_1_fixed_point.html</filename>
    <templarg></templarg>
    <templarg>FracSize</templarg>
    <member kind="enumvalue">
      <name>One</name>
      <anchorfile>classmath_1_1_fixed_point.html</anchorfile>
      <anchor>afb9ffb124164499d6fc72ea9bf3b3b4ba101ca4d0e2bd09a98c0c1b6d9dbf4bc4</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>IntBits</name>
      <anchorfile>classmath_1_1_fixed_point.html</anchorfile>
      <anchor>afb9ffb124164499d6fc72ea9bf3b3b4ba32189caa21c936f86842c1cefdaa5a0e</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>FracBits</name>
      <anchorfile>classmath_1_1_fixed_point.html</anchorfile>
      <anchor>afb9ffb124164499d6fc72ea9bf3b3b4ba60a4aa907715b873db9d19b358a0a939</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>MaxVal</name>
      <anchorfile>classmath_1_1_fixed_point.html</anchorfile>
      <anchor>afb9ffb124164499d6fc72ea9bf3b3b4ba5d030913564b104c52ac5438cbcad22b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>MaxFrac</name>
      <anchorfile>classmath_1_1_fixed_point.html</anchorfile>
      <anchor>afb9ffb124164499d6fc72ea9bf3b3b4ba0dc41bdcb6d57c378c2ec1c85956c87b</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>Epsilon</name>
      <anchorfile>classmath_1_1_fixed_point.html</anchorfile>
      <anchor>afb9ffb124164499d6fc72ea9bf3b3b4bae9031c5a36a4af7186978efbde7d4a49</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>FixedPoint</name>
      <anchorfile>classmath_1_1_fixed_point.html</anchorfile>
      <anchor>a627e839aef1a68b0ae0219a54f89eb5c</anchor>
      <arglist>(const BaseT &amp;v)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>FixedPoint</name>
      <anchorfile>classmath_1_1_fixed_point.html</anchorfile>
      <anchor>a6de12ac2331aaca125f6dc03bd8bfcfe</anchor>
      <arglist>(const BaseT &amp;whole, const BaseT &amp;frac)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>FixedPoint</name>
      <anchorfile>classmath_1_1_fixed_point.html</anchorfile>
      <anchor>a06ce18a1694435b98f5526d9ae1863c0</anchor>
      <arglist>(const BaseT &amp;whole, const BaseT &amp;nomin, const BaseT &amp;denom)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>operator double</name>
      <anchorfile>classmath_1_1_fixed_point.html</anchorfile>
      <anchor>a8268ce8f1bb33ad9327abff79e98ab18</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>BaseT</type>
      <name>Int</name>
      <anchorfile>classmath_1_1_fixed_point.html</anchorfile>
      <anchor>a4403f060bd32b2bb2eff60f75c24fad0</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="function">
      <type>BaseT</type>
      <name>Frac</name>
      <anchorfile>classmath_1_1_fixed_point.html</anchorfile>
      <anchor>af8e26ad7c93ca6285123642092d17f49</anchor>
      <arglist>() const </arglist>
    </member>
    <member kind="variable">
      <type>BaseT</type>
      <name>value</name>
      <anchorfile>classmath_1_1_fixed_point.html</anchorfile>
      <anchor>a75ae065a1f0bbbd22e379a95a6f424b6</anchor>
      <arglist></arglist>
    </member>
  </compound>
</tagfile>
