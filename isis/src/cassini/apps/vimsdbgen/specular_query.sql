select name, observationid, line, sample, latitude, longitude,
       phase, incidence, emission,
       distance, lineres, sampres,
       spectrum[248] as band_248,
       abs(incidence - emission) as i_e_diff,
       abs(phase - (incidence + emission)) as g_eq_i_e
       from vimspixel
       where (abs(phase - (incidence + emission))  < 0.1)
        and (abs(incidence - emission) < 5.0) ;
--        and (spectrum[248] > 1.0);

