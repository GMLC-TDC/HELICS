function v = helics_flag_restrictive_time_policy()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 35);
  end
  v = vInitialized;
end
