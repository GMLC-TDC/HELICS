function v = helics_flag_ignore_time_mismatch_warnings()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856901);
  end
  v = vInitialized;
end
