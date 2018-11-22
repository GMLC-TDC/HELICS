function v = helics_flag_uninterruptible()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183056);
  end
  v = vInitialized;
end
