#include "Conf.h"

void Conf::massCenter(Topo* topo) {
    syscm.x = 0;
    syscm.y = 0;
    syscm.z = 0;
    for (unsigned long i=0; i < particleStore.size(); i++) {
        //using periodic boundary conditions
        syscm.x += (particleStore[i]->pos.x - anInt(particleStore[i]->pos.x) ) *
            topo->ia_params[particleStore[i]->type][particleStore[i]->type].volume;
        syscm.y += (particleStore[i]->pos.y - anInt(particleStore[i]->pos.y) ) *
            topo->ia_params[particleStore[i]->type][particleStore[i]->type].volume;
        syscm.z += (particleStore[i]->pos.z - anInt(particleStore[i]->pos.z) ) *
            topo->ia_params[particleStore[i]->type][particleStore[i]->type].volume;
    }
    syscm.x /= sysvolume;
    syscm.y /= sysvolume;
    syscm.z /= sysvolume;
    return;
}

void Conf::partVecInit(Topo* topo) {
    for(int i = 0; i < (long)particleStore.size(); i++){
        if ( topo->ia_params[particleStore[i]->type][particleStore[i]->type].geotype[0]  < SP)
            particleStore[i]->init(&(topo->ia_params[particleStore[i]->type][particleStore[i]->type]));
    }
}

int Conf::overlap(Particle *part1, Particle *part2, Ia_param ia_params[][MAXT]) {

    double b, c, d, e, f;   /* Coefficients in distance quadratic */
    double boundary;     /* Half length of central boundary zone of quadratic */
    double det;
    double halfl;        /* Half length of cylinder */
    double s0, t0;       /* det times location of min separation of infinite lines */
    double ss, tt;       /* Location of min separation of line segments */
    Vector r_cm;  /* Vector between centres of mass */
    double dist;         /* Distance between particles*/
    Vector distvec; /* Distance vector between particles*/

    r_cm = image(&part1->pos, &part2->pos, &box);
    if ((part1->type >= SP) && (part2->type >= SP)) { /*we have two spheres - most common, do nothing*/
        dist=sqrt(DOT(r_cm,r_cm));
    } else {
        if ((ia_params[part1->type][part2->type].geotype[0] < SP) && (ia_params[part1->type][part2->type].geotype[1] < SP)) { /*we have two spherocylinders*/
            /*finding closes contact between them*/
            b = -DOT(part1->dir, part2->dir);
            d =  DOT(part1->dir, r_cm);
            e = -DOT(part2->dir, r_cm);
            f =  DOT(r_cm, r_cm);
            det = 1.0 - b*b;
            //halfl = length / 2.0;
            // Just take the mean
            halfl = ia_params[part1->type][part2->type].half_len[0] = ia_params[part1->type][part2->type].half_len[1];
            halfl /= 2;
            boundary = det * halfl;
            /* Location of smallest separation of the infinite lines */
            s0 = b*e - d;
            t0 = b*d - e;
            /* Location of smallest separation of line segments */
            if (s0 >= boundary) {
                if (t0 >= boundary) {
                    /* Region 2 */
                    if ( d + halfl + halfl*b < 0.0 ) {
                        ss = halfl;
                        tt = linemin( -ss*b - e, halfl );
                    } else {
                        tt = halfl;
                        ss = linemin( -tt*b - d, halfl );
                    }
                } else if (t0 >= -boundary) {
                    /* Region 1 */
                    ss = halfl;
                    tt = linemin( -ss*b - e, halfl );
                } else {
                    /* Region 8 */
                    if ( d + halfl - halfl*b < 0.0 ) {
                        ss = halfl;
                        tt = linemin( -ss*b - e, halfl );
                    } else {
                        tt = -halfl;
                        ss = linemin( -tt*b - d, halfl );
                    }
                }
            } else if (s0 >= -boundary) {
                if (t0 >= boundary) {
                    /* Region 3 */
                    tt = halfl;
                    ss = linemin( -tt*b - d, halfl );
                } else if (t0 >= -boundary) {
                    /* Region 0 */
                    ss = s0/det;
                    tt = t0/det;
                } else {
                    /* Region 7 */
                    tt = -halfl;
                    ss = linemin( -tt*b - d, halfl );
                }
            } else {
                if (t0 >= boundary) {
                    /* Region 4 */
                    if ( d - halfl + halfl*b > 0.0 ) {
                        ss = -halfl;
                        tt = linemin( -ss*b - e, halfl );
                    } else {
                        tt = halfl;
                        ss = linemin( -tt*b - d, halfl );
                    }
                } else if (t0 >= -boundary) {
                    /* Region 5 */
                    ss = -halfl;
                    tt = linemin( -ss*b - e, halfl );
                } else {
                    /* Region 6 */
                    if ( d - halfl - halfl*b > 0.0 ) {
                        ss = -halfl;
                        tt = linemin( -ss*b - e, halfl );
                    } else {
                        tt = -halfl;
                        ss = linemin( -tt*b - d, halfl );
                    }
                }
            }
            /*ss snd tt are Location of min separation of line segments */
            dist=sqrt(f + ss*ss + tt*tt + 2.0*(ss*d + tt*e + ss*tt*b));
        } else {
            if (ia_params[part1->type][part2->type].geotype[0] < SP) { /*We have one spherocylinder -it is first one*/
                //halfl=length/2;/*finding closest vector from sphyrocylinder to sphere*/
                halfl=ia_params[part1->type][part2->type].half_len[0];/*finding closest vector from sphyrocylinder to sphere*/
                c = DOT(part1->dir,r_cm);
                if (c >= halfl) d = halfl;
                else {
                    if (c > -halfl) d = c;
                    else d = -halfl;
                }
                distvec.x = - r_cm.x + part1->dir.x * d;
                distvec.y = - r_cm.y + part1->dir.y * d;
                distvec.z = - r_cm.z + part1->dir.z * d;
                dist=sqrt(DOT(distvec,distvec));
            } else { /*lst option first one is sphere second one spherocylinder*/
                //halfl=length/2; /*finding closest vector from sphyrocylinder to sphere*/
                halfl=ia_params[part1->type][part2->type].half_len[1];/*finding closest vector from sphyrocylinder to sphere*/
                c = DOT(part2->dir,r_cm);
                if (c >= halfl) d = halfl;
                else {
                    if (c > -halfl) d = c;
                    else d = -halfl;
                }
                distvec.x = r_cm.x - part2->dir.x * d;
                distvec.y = r_cm.y - part2->dir.y * d;
                distvec.z = r_cm.z - part2->dir.z * d;
                dist=sqrt(DOT(distvec,distvec));
            }
        }
    }

    /* Overlap exists if smallest separation is less than diameter of cylinder */
    if (dist < ia_params[part1->type][part2->type].sigma*0.5 ) {
        return 1;
    } else {
        return 0;
    }
}




bool Conf::overlapAll(Particle* target, Ia_param ia_params[][MAXT]) {
    for (unsigned long i=0; i<particleStore.size(); i++) {
        if (particleStore[i] != target) {
            if ( overlap(target, particleStore[i], ia_params) ) {
                return true;
            }
        }
    }
    return false;
}




int Conf::checkall(Ia_param ia_params[][MAXT]) {
    unsigned long i, j;

    for (i=0; i<particleStore.size()-1; i++) {
        for (j=i+1; j<particleStore.size(); j++) {
            if ( overlap(particleStore[i], particleStore[j], ia_params) ) {
                return 1;
            }
        }
    }
    return 0;
}

