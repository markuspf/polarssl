/*
 *  RSASSA-PSS/SHA-1 signature creation program
 *
 *  Copyright (C) 2006-2010, Brainspark B.V.
 *
 *  This file is part of PolarSSL (http://www.polarssl.org)
 *  Lead Maintainer: Paul Bakker <polarssl_maintainer at polarssl.org>
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include <string.h>
#include <stdio.h>

#include "polarssl/havege.h"
#include "polarssl/md.h"
#include "polarssl/rsa.h"
#include "polarssl/sha1.h"
#include "polarssl/x509.h"

int main( int argc, char *argv[] )
{
    FILE *f;
    int ret;
    rsa_context rsa;
    havege_state hs;
    unsigned char hash[20];
    unsigned char buf[512];
    char filename[512];

    ret = 1;

    if( argc != 3 )
    {
        printf( "usage: rsa_sign_pss <key_file> <filename>\n" );

#ifdef WIN32
        printf( "\n" );
#endif

        goto exit;
    }

    printf( "\n  . Reading private key from '%s'", argv[1] );
    fflush( stdout );

    havege_init( &hs );
    rsa_init( &rsa, RSA_PKCS_V21, POLARSSL_MD_SHA1 );

    if( ( ret = x509parse_keyfile( &rsa, argv[1], "" ) ) != 0 )
    {
        ret = 1;
        printf( " failed\n  ! Could not open '%s'\n", argv[1] );
        goto exit;
    }

    /*
     * Compute the SHA-1 hash of the input file,
     * then calculate the RSA signature of the hash.
     */
    printf( "\n  . Generating the RSA/SHA-1 signature" );
    fflush( stdout );

    if( ( ret = sha1_file( argv[2], hash ) ) != 0 )
    {
        printf( " failed\n  ! Could not open or read %s\n\n", argv[2] );
        goto exit;
    }

    if( ( ret = rsa_pkcs1_sign( &rsa, havege_rand, &hs, RSA_PRIVATE, SIG_RSA_SHA1,
                                20, hash, buf ) ) != 0 )
    {
        printf( " failed\n  ! rsa_pkcs1_sign returned %d\n\n", ret );
        goto exit;
    }

    /*
     * Write the signature into <filename>-sig.txt
     */
    snprintf( filename, 512, "%s.sig", argv[2] );

    if( ( f = fopen( filename, "wb+" ) ) == NULL )
    {
        ret = 1;
        printf( " failed\n  ! Could not create %s\n\n", filename );
        goto exit;
    }

    if( fwrite( buf, 1, rsa.len, f ) != (size_t) rsa.len )
    {
        printf( "failed\n  ! fwrite failed\n\n" );
        goto exit;
    }

    fclose( f );

    printf( "\n  . Done (created \"%s\")\n\n", filename );

exit:

#ifdef WIN32
    printf( "  + Press Enter to exit this program.\n" );
    fflush( stdout ); getchar();
#endif

    return( ret );
}