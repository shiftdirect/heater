/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2018  Ray Jones <ray@mrjones.id.au>
 * Copyright (C) 2018  James Clark
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */

#ifndef __MACROS_H__
#define __MACROS_H__

#define LOWERLIMIT(A, B) if((A) < (B)) (A) = (B)
#define UPPERLIMIT(A, B) if((A) > (B)) (A) = (B)
#define WRAPUPPERLIMIT(A, B, C) if((A) > (B)) (A) = (C)        
#define WRAPLOWERLIMIT(A, B, C) if((A) < (B)) (A) = (C)        
#define WRAPLIMITS(A, B, C) if((A) < (B)) (A) = (C) ; if((A) > (C)) (A) = (B)               
#define INBOUNDS(TST, MIN, MAX) (((TST) >= (MIN)) && ((TST) <= (MAX)))
#define BOUNDSLIMIT(A, B, C) if((A) < (B)) (A) = (B); if((A) > (C)) (A) = (C)

#endif