function v = helics_flag_uninterruptible()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183045);
  end
  v = vInitialized;
end
