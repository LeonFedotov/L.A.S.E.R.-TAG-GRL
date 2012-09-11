#ifndef _OF_VEC2f
#define _OF_VEC2f



class ofVec2f{

public:
	
	
    //----------------  Constructors
    
    
    /**
     * Constructor. Defaults to zero,zero,zero.
     *
     * @param  _x initial x coordinate.
     * @param  _y initial y coordinate.
     */
    ofVec2f( const float _x = 0.0f, const float _y = 0.0f )
        { x = _x;  y = _y; }
	
    /**
     * Constructor. Copies value in vec.
     *
     * @param  vec source of initial coordinates to copy.
     */
    ofVec2f( const ofVec2f &vec )  
        { x = vec.x;  y = vec.y; }
    
    
    
    //---------------- Data Members
    
    /**
     * x-coordinate
    */
    float x;
    
    /**
     * y-coordinate
    */
    float y;
    
   
    //---------------- Methods
    
    /**
     * Set x,y,z coordinates of vector.
     *
     * @param  _x  x coordinate.
     * @param  _y  y coordinate.
     */
    void set( 	float _x, float _y )
        { x = _x;  y = _y;  }
    
    /**
     * Set x,y coordinates to the values in a vector object.
     *
     * @param vec  vector object to copy.
     */
     void set( const ofVec2f &vec )
       { x = vec.x;  y = vec.y; }

    
    
    /**
	  * Array access operator
	  * @param i Array index
	  * @return For i = 0, reference to x coordinate, i = 1
	  * reference to y
	  * x coordinate.
	  */
    float &operator[]( const int &i )           
        { switch(i) {
          case 0:  return x;
          case 1:  return y;
          default: return x; } }

    
    
    /**
     * Addition operator.
     * @param vec the vector to add
     * @return this + vec
     */  
    ofVec2f operator+( const ofVec2f & vec ) const          
    { 
        return ofVec2f( x+vec.x, y+vec.y); 
    } 
    
    /**
     * Add a vector into this
     * @param vec the vector to add
     * @return this 
     */
    ofVec2f &operator+=( const ofVec2f & vec )              
    { 
        x+=vec.x; y+=vec.y;
        return *this; 
    } 

    /**
     * Subtraction operator.
     * @return -this
     */ 
    ofVec2f operator-() const                               
    { 
        return ofVec2f(-x, -y); 
    } 
    
    /**
     * Subtraction operator.
     * @param vec to subtract
     * @return this - vec
     */ 
    ofVec2f operator-( const ofVec2f & vec ) const          
    { 
        return ofVec2f(x-vec.x, y-vec.y); 
    } 
    
    /**
     * Subtract a vector from this.
     * @param vec to subtract
     * @return this 
     */ 
    ofVec2f &operator-=( const ofVec2f & vec )              
    { 
        x-=vec.x; y-=vec.y;
        return *this; 
    } 
    
    
    /**
     * Multiplication operator.
     * @param vec to multiply by
     * @return this * vec
     */ 
    ofVec2f operator*( const ofVec2f & vec ) const
    { 
        return ofVec2f(x*vec.x, y*vec.y); 
    } 
    
    /**
     * Multiplication operator.
     * @param vec to multiply by
     * @return this
     */ 
     ofVec2f &operator*=( const ofVec2f & vec )      
    { 
        x*=vec.x; y*=vec.y;
        return *this; 
    } 

    
     /**
     * Scalar Multiplication.
     * @param f value to multiply by 
     * @param vec vector to scale
     * @return vector scaled by value in f
     */ 
    friend ofVec2f operator*( const float f, const ofVec2f &vec )  
    { 
        return ofVec2f(vec.x*f, vec.y*f); 
    } 
    /**
     * Division operator.
     * @param vec to divide by
     * @return this / vec
     */ 
    ofVec2f operator/( const ofVec2f & vec ) const  
    { 
        return ofVec2f(x/vec.x, y/vec.y);
    } 
    
    /**
     * Division operator.
     * @param vec to divide by
     * @return this
     */ 
    ofVec2f &operator/=( const ofVec2f & vec )      
    { 
        x/=vec.x; y/=vec.y;
        return *this; 
    } 

    /**
     * Scalar Multiplication. 
     * @param f value to multiply by. 
     * @return value of this scaled by value in f
     */ 
    ofVec2f operator*( const float f ) const                
    { 
        return ofVec2f(x*f, y*f);
    } 
    
      /**
     * Scalar Multiplication.
     * @param f value to multiply by 
     * @return this
     */ 
     ofVec2f &operator*=( const float f )                    
    { 
        x*=f; y*=f;
        return *this; 
    } 

    
    /**
     * Scalar Division. 
     * @param f value to divide by. 
     */ 
    ofVec2f operator/( const float f ) const      
    { 
        return ofVec2f(x/f, y/f);
    } 
    
     /**
     * Scalar Division. 
     * @param f value to divide by. 
     */ 
    ofVec2f &operator/=( const float f )          
    { 
        x/=f; y/=f;
        return *this; 
    } 

     /**
     * Calculate the length (magnitude) of the vector. 
     * @return length
    */
    float length() const                
    { 
        return (float)sqrt(x*x + y*y); 
    } 
    
    /**
     * Calculate the squared length of the vector. 
     * This should be faster than length()
     * @return length squared
    */
    float lengthSquared() const                
    { 
        return (float)(x*x + y*y); 
    } 
    
    /**
     * Normalizes this 
     * @return this
    */
    ofVec2f &normalize()                  
    { 
			float f = length();
			if( f > 0){ 
				x/=f; 
				y/=f; 
			}
        
		  return *this; 
	} 
    
    /**
     * Calculates normal without modifying this. 
     * @return normalized value
    */
    ofVec2f norm_value() const 
    { 
        float f = length();
        if( f > 0) 
            return ofVec2f(x/f, y/f); 
        else 
            return ofVec2f(0, 0); 
    } 

	/**
     * Limit the length of this vector
     * @param max the maximum length to limit this vector by
     */  
	 ofVec2f limit(float max) {
		if (length() > max) {
			normalize();
			x*=max; y*=max;
		}
		return *this; 
	}
	
	
    /**
    * Calculate angele between this and vector.
    * @param vec
    * @return angle in radians
    */
    float angle( const ofVec2f &vec ) const
    {
          return atan2( vec.y - y, vec.x - x );
    }
    
     /**
    * Calculate perpendicular vector for this and vector.
    * @param vec
    * @return normal vector of perpendcular 
    */      
    ofVec2f perpendicular( const ofVec2f &vec ) const
    {
            float a = -atan2( vec.y - y, vec.x - x );
            return ofVec2f( sin(a), cos(a) ); 

    }
            

	
};

#endif
