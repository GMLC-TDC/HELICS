function v = helics_flag_slow_responding()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 37);
  end
  v = vInitialized;
end
