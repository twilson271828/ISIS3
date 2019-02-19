select name, line, sample, latitude, longitude, phase, incidence, emission,
      spectrum[248] as band_248, abs(incidence - emission) as i_e_diff,
      abs(phase - (incidence + emission)) as g_eq_i_e  from vimspixel
      where
--     (spectrum[248] > 1.0) and
             (abs(phase - (incidence + emission))  < 5.0)
        and (abs(incidence - emission) < 5.0);
