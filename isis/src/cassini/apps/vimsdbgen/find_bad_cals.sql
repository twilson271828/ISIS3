
select name, observationid, line, sample, latitude, longitude,
       phase, incidence, emission,
       distance, lineres, sampres,
       spectrum[71] as band_71,
       spectrum[248] as band_248
       from vimspixel
       where 
            (incidence < 89.0) 
        AND (emission  < 80.0)
        AND ( (spectrum[71] > 0.4) OR (spectrum[248] > 0.4) )
