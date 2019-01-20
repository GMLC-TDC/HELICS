function v = other_error_type()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812680);
  end
  v = vInitialized;
end
