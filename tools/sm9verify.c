﻿/*
 * Copyright (c) 2021 - 2021 The GmSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the GmSSL Project.
 *    (http://gmssl.org/)"
 *
 * 4. The name "GmSSL Project" must not be used to endorse or promote
 *    products derived from this software without prior written
 *    permission. For written permission, please contact
 *    guanzhi1980@gmail.com.
 *
 * 5. Products derived from this software may not be called "GmSSL"
 *    nor may "GmSSL" appear in their names without prior written
 *    permission of the GmSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the GmSSL Project
 *    (http://gmssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE GmSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE GmSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gmssl/sm9.h>
#include <gmssl/error.h>


static const char *options = "[-in file] -pubmaster file -id str -sig file";

int sm9verify_main(int argc, char **argv)
{
	int ret = 1;
	char *prog = argv[0];
	char *infile = NULL;
	char *mpkfile = NULL;
	char *id = NULL;
	char *sigfile = NULL;
	FILE *infp = stdin;
	FILE *mpkfp = NULL;
	FILE *sigfp = NULL;
	SM9_SIGN_MASTER_KEY mpk;
	SM9_SIGN_CTX ctx;
	uint8_t buf[4096];
	ssize_t len;
	uint8_t sig[SM9_SIGNATURE_SIZE];
	ssize_t siglen;

	argc--;
	argv++;

	if (argc < 1) {
		fprintf(stderr, "usage: %s %s\n", prog, options);
		return 1;
	}

	while (argc > 0) {
		if (!strcmp(*argv, "-help")) {
			fprintf(stdout, "usage: %s %s\n", prog, options);
			return 0;
		} else if (!strcmp(*argv, "-in")) {
			if (--argc < 1) goto bad;
			infile = *(++argv);
			if (!(infp = fopen(infile, "r"))) {
				error_print();
				goto end;
			}
		} else if (!strcmp(*argv, "-pubmaster")) {
			if (--argc < 1) goto bad;
			mpkfile = *(++argv);
			if (!(mpkfp = fopen(mpkfile, "r"))) {
				error_print();
				goto end;
			}
		} else if (!strcmp(*argv, "-id")) {
			if (--argc < 1) goto bad;
			id = *(++argv);
		} else if (!strcmp(*argv, "-sig")) {
			if (--argc < 1) goto bad;
			sigfile = *(++argv);
			if (!(sigfp = fopen(sigfile, "r"))) {
				error_print();
				goto end;
			}
		} else {
bad:
			fprintf(stderr, "%s: illegal option '%s'\n", prog, *argv);
			return 1;
		}

		argc--;
		argv++;
	}

	if (!mpkfile || !id || !sigfile) {
		error_print();
		goto end;
	}

	if (sm9_sign_master_public_key_from_pem(&mpk, mpkfp) != 1) {
		error_print();
		goto end;
	}

	if ((siglen = fread(sig, 1, sizeof(sig), sigfp)) <= 0) {
		error_print();
		goto end;
	}

	if (sm9_verify_init(&ctx) != 1) {
		error_print();
		goto end;
	}
	while ((len = fread(buf, 1, sizeof(buf), infp)) > 0) {
		if (sm9_verify_update(&ctx, buf, len) != 1) {
			error_print();
			goto end;
		}
	}
	if ((ret = sm9_verify_finish(&ctx, sig, siglen, &mpk, id, strlen(id))) != 1) {
		error_print();
		goto end;
	}
	printf("%s %s\n", prog, ret ? "success" : "failure");

end:
	if (infile && infp) fclose(infp);
	if (mpkfile && mpkfp) fclose(mpkfp);
	if (sigfile && sigfp) fclose(sigfp);
	return ret;
}








